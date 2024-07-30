/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#include <rtps/entities/ReaderProxy.h>
#include <rtps/entities/Writer.h>

#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "rtps/ThreadPool.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/Log.h"
#include "rtps/utils/udpUtils.h"

using rtps::CacheChange;
using rtps::SequenceNumber_t;
using rtps::StatelessWriterT;

#if SLW_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef SLW_LOG
#include "rtps/utils/strutils.h"
#define SLW_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatelessWriter %s] ", &this->m_attributes.topicName[0]);         \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#endif
#else
#define SLW_LOG(...) //
#endif

template <class NetworkDriver>
StatelessWriterT<NetworkDriver>::~StatelessWriterT() {
  //  if(sys_mutex_valid(&m_mutex)){
  //    sys_mutex_free(&m_mutex);
  //  }
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::init(TopicData attributes,
                                           TopicKind_t topicKind,
                                           ThreadPool *threadPool,
                                           NetworkDriver &driver,
                                           bool enfUnicast) {

  m_attributes = attributes;

  if (m_mutex == nullptr) {
    if (!createMutex(&m_mutex)) {
#if SLW_VERBOSE
      SLW_LOG("Failed to create mutex \n");
#endif
      return false;
    }
  }

  mp_threadPool = threadPool;
  m_srcPort = attributes.unicastLocator.port;
  m_enforceUnicast = enfUnicast;

  m_topicKind = topicKind;
  m_nextSequenceNumberToSend = {0, 1};
  m_is_initialized_ = true;

  m_proxies.clear();
  m_history.clear();

  m_transport = &driver;

  return true;
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::reset() {
  m_is_initialized_ = false;
}

template <typename NetworkDriver>
const CacheChange *StatelessWriterT<NetworkDriver>::newChange(
    rtps::ChangeKind_t kind, const uint8_t *data, DataSize_t size,
    bool inLineQoS, bool markDisposedAfterWrite) {
  INIT_GUARD();
  if (isIrrelevant(kind)) {
    return nullptr;
  }
  Lock lock(m_mutex);
  if (!m_is_initialized_) {
    return nullptr;
  }

  if (m_history.isFull()) {
    SequenceNumber_t newMin = ++SequenceNumber_t(m_history.getSeqNumMin());
    if (m_nextSequenceNumberToSend < newMin) {
      m_nextSequenceNumberToSend =
          newMin; // Make sure we have the correct sn to send
    }
    SLW_LOG("History is full, dropping oldest %s\r\n", this->m_attributes.topicName);
  }

  auto *result = m_history.addChange(data, size);
  if (mp_threadPool != nullptr) {
    mp_threadPool->addWorkload(this);
  }

  SLW_LOG("Adding new data.\n");
  return result;
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::removeFromHistory(
    const SequenceNumber_t &s) {
  return false; // Stateless Writers currently do not support deletion from
                // history
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::setAllChangesToUnsent() {
  INIT_GUARD();
  Lock lock(m_mutex);

  m_nextSequenceNumberToSend = m_history.getSeqNumMin();

  if (mp_threadPool != nullptr) {
    mp_threadPool->addWorkload(this);
  }
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::onNewAckNack(
    const SubmessageAckNack & /*msg*/, const GuidPrefix_t &sourceGuidPrefix) {
  INIT_GUARD();
  // Too lazy to respond
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::progress() {
  INIT_GUARD();
  // TODO smarter packaging e.g. by creating MessageStruct and serialize after
  // adjusting values Reusing the pbuf is not possible. See
  // https://www.nongnu.org/lwip/2_1_x/raw_api.html (Zero-Copy MACs)

  if (m_proxies.getNumElements() == 0) {
    SLW_LOG("No Proxy!\n");
  }

  for (const auto &proxy : m_proxies) {

    SLW_LOG("Progess.\n");
    // Do nothing, if someone else sends for me... (Multicast)
    if (proxy.useMulticast || !proxy.suppressUnicast || m_enforceUnicast) {
      PacketInfo info;
      info.srcPort = m_srcPort;

      MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
      MessageFactory::addSubMessageTimeStamp(info.buffer);

      {
        Lock lock(m_mutex);
        const CacheChange *next =
            m_history.getChangeBySN(m_nextSequenceNumberToSend);
        if (next == nullptr) {
          SLW_LOG("Couldn't get a new CacheChange with SN "
                  "(%i,%i)\n",
                  m_nextSequenceNumberToSend.high,
                  m_nextSequenceNumberToSend.low);
          return;
        } else {
          SLW_LOG("Sending change with SN (%i,%i)\n",
                  m_nextSequenceNumberToSend.high,
                  m_nextSequenceNumberToSend.low);
        }

        // Set EntityId to UNKNOWN if using multicast, because there might be
        // different ones...
        // TODO: mybe enhance by using UNKNOWN only if ids are really different
        EntityId_t reid;
        if (proxy.useMulticast && !m_enforceUnicast && proxy.unknown_eid) {
          reid = ENTITYID_UNKNOWN;
        } else {
          reid = proxy.remoteReaderGuid.entityId;
        }
        MessageFactory::addSubMessageData(info.buffer, next->data, false,
                                          next->sequenceNumber,
                                          m_attributes.endpointGuid.entityId,
                                          reid); // TODO
      }

      // Just usable for IPv4
      // Decide which locator to be used unicast/multicast

      if (proxy.useMulticast && !m_enforceUnicast) {
        info.destAddr = proxy.remoteMulticastLocator.getIp4Address();
        info.destPort = (Ip4Port_t)proxy.remoteMulticastLocator.port;
      } else {
        info.destAddr = proxy.remoteLocator.getIp4Address();
        info.destPort = (Ip4Port_t)proxy.remoteLocator.port;
      }
      
      m_transport->sendPacket(info);
    }
  }

  m_history.removeUntilIncl(m_nextSequenceNumberToSend);
  ++m_nextSequenceNumberToSend;
}

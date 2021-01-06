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

#define SLW_VERBOSE 0

#if SLW_VERBOSE
#include "rtps/utils/printutils.h"
#endif

template <class NetworkDriver>
StatelessWriterT<NetworkDriver>::~StatelessWriterT() {
  // if(sys_mutex_valid(&m_mutex)){
  sys_mutex_free(&m_mutex);
  //}
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::init(TopicData attributes,
                                           TopicKind_t topicKind,
                                           ThreadPool *threadPool,
                                           NetworkDriver &driver) {
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
#if SLW_VERBOSE
    Log::printLine("SFW:Failed to create mutex \n");
#endif
    return false;
  }

  m_attributes = attributes;
  m_packetInfo.srcPort = attributes.unicastLocator.port;
  m_topicKind = topicKind;
  mp_threadPool = threadPool;
  m_transport = &driver;

  m_is_initialized_ = true;
  return true;
}

template <class NetworkDriver>
bool StatelessWriterT<NetworkDriver>::addNewMatchedReader(
    const ReaderProxy &newProxy) {
#if SLW_VERBOSE
  printf("StatefulWriter[%s]: New reader added with id: ",
         &this->m_attributes.topicName[0]);
  printGuid(newProxy.remoteReaderGuid);
  printf("\n");
#endif
  bool success = m_proxies.add(newProxy);
  manageSendOptions();
  return success;
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::manageSendOptions() {
#if SLW_VERBOSE
  printf("Search for Multicast Partners!\n");
#endif
  for (auto &proxy : m_proxies) {
    if (proxy.remoteMulticastLocator.kind == LocatorKind_t::LOCATOR_KIND_INVALID) {
      proxy.suppressUnicast = false;
      proxy.useMulticast = false;
    } else {
      bool found = false;
      for (auto &avproxy : m_proxies) {
        if (avproxy.remoteMulticastLocator.kind == LocatorKind_t::LOCATOR_KIND_UDPv4 && 
            avproxy.remoteMulticastLocator.getIp4Address().addr == 
            proxy.remoteMulticastLocator.getIp4Address().addr &&
            avproxy.remoteLocator.getIp4Address().addr !=
            proxy.remoteLocator.getIp4Address().addr){
          if (avproxy.suppressUnicast == false) {
            avproxy.useMulticast = false;
            avproxy.suppressUnicast = true;
            proxy.useMulticast = true;
            proxy.suppressUnicast = true;
#if SLW_VERBOSE
            printf("Found Multicast Partner!\n");
#endif
          }
          found = true;
        }
      }
      if (!found) {
        proxy.useMulticast = false;
        proxy.suppressUnicast = false;
      }
    }
    
  }
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::removeReader(const Guid &guid) {
  auto isElementToRemove = [&](const ReaderProxy &proxy) {
    return proxy.remoteReaderGuid == guid;
  };
  auto thunk = [](void *arg, const ReaderProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

template <typename NetworkDriver>
const CacheChange *StatelessWriterT<NetworkDriver>::newChange(
    rtps::ChangeKind_t kind, const uint8_t *data, DataSize_t size) {
  if (isIrrelevant(kind)) {
    return nullptr;
  }
  Lock lock(m_mutex);

  if (m_history.isFull()) {
    SequenceNumber_t newMin = ++SequenceNumber_t(m_history.getSeqNumMin());
    if (m_nextSequenceNumberToSend < newMin) {
      m_nextSequenceNumberToSend =
          newMin; // Make sure we have the correct sn to send
    }
  }

  auto *result = m_history.addChange(data, size);
  if (mp_threadPool != nullptr) {
    mp_threadPool->addWorkload(this);
  }

#if SLW_VERBOSE
  printf("StatelessWriter[%s]: Adding new data.\n",
         this->m_attributes.topicName);
#endif
  return result;
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::setAllChangesToUnsent() {
  Lock lock(m_mutex);

  m_nextSequenceNumberToSend = m_history.getSeqNumMin();

  if (mp_threadPool != nullptr) {
    mp_threadPool->addWorkload(this);
  }
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::onNewAckNack(
    const SubmessageAckNack & /*msg*/, const GuidPrefix_t &sourceGuidPrefix) {
  // Too lazy to respond
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::isIrrelevant(ChangeKind_t kind) const {
  // Right now we only allow alive changes
  // return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY
  // && kind != ChangeKind_t::ALIVE);
  return kind != ChangeKind_t::ALIVE;
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::progress() {
  // TODO smarter packaging e.g. by creating MessageStruct and serialize after
  // adjusting values Reusing the pbuf is not possible. See
  // https://www.nongnu.org/lwip/2_1_x/raw_api.html (Zero-Copy MACs)

  for (const auto &proxy : m_proxies) {

#if SLW_VERBOSE
    printf("StatelessWriter[%s]: Progess.\n", this->m_attributes.topicName);
#endif
    // Do nothing, if someone else sends for me... (Multicast)
    if(proxy.useMulticast || proxy.suppressUnicast == false) {
      PacketInfo info;
      info.srcPort = m_packetInfo.srcPort;

      MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
      MessageFactory::addSubMessageTimeStamp(info.buffer);

      {
        Lock lock(m_mutex);
        const CacheChange *next =
            m_history.getChangeBySN(m_nextSequenceNumberToSend);
        if (next == nullptr) {
#if SLW_VERBOSE
          printf("StatelessWriter[%s]: Couldn't get a new CacheChange with SN "
                "(%i,%i)\n",
                &m_attributes.topicName[0], m_nextSequenceNumberToSend.high,
                m_nextSequenceNumberToSend.low);
#endif
          return;
        } else {
#if SLW_VERBOSE
          printf("StatelessWriter[%s]: Sending change with SN (%i,%i)\n",
                &m_attributes.topicName[0], m_nextSequenceNumberToSend.high,
                m_nextSequenceNumberToSend.low);
#endif
        }

        // Set EntityId to UNKNOWN if using multicast, because there might be different ones...
        // TODO: mybe enhance by using UNKNOWN only if ids are really different
        EntityId_t reid;
        if(proxy.useMulticast) {
          reid = ENTITYID_UNKNOWN;
        } else {
          reid = proxy.remoteReaderGuid.entityId;
        }
        MessageFactory::addSubMessageData(
            info.buffer, next->data, false, next->sequenceNumber,
            m_attributes.endpointGuid.entityId,
            reid); // TODO
      }

      // Just usable for IPv4
      // Decide which locator to be used unicast/multicast

      if(proxy.useMulticast) {
        const Locator &locator = proxy.remoteMulticastLocator;
        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t)locator.port;
      } else {
        const Locator &locator = proxy.remoteLocator;
        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t)locator.port;
      }

      m_transport->sendPacket(info);
    }
  }

  ++m_nextSequenceNumberToSend;
}

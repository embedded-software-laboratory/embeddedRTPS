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
#include "rtps/utils/printutils.h"
#define SLW_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatelessWriter %s] ", &this->m_attributes.topicName[0]);         \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
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
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
#if SLW_VERBOSE
    SLW_LOG("Failed to create mutex \n");
#endif
    return false;
  }

  m_attributes = attributes;
  m_packetInfo.srcPort = attributes.unicastLocator.port;
  m_topicKind = topicKind;
  mp_threadPool = threadPool;
  m_transport = &driver;
  m_enforceUnicast = enfUnicast;

  m_is_initialized_ = true;
  return true;
}

template <class NetworkDriver>
bool StatelessWriterT<NetworkDriver>::addNewMatchedReader(
    const ReaderProxy &newProxy) {
#if SLW_VERBOSE && RTPS_GLOBAL_VERBOSE
  SLW_LOG("New reader added with id: ");
  printGuid(newProxy.remoteReaderGuid);
#endif
  bool success = m_proxies.add(newProxy);
  if (!m_enforceUnicast) {
    manageSendOptions();
  }
  return success;
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::manageSendOptions() {
  SLW_LOG("Search for Multicast Partners!\n");
  for (auto &proxy : m_proxies) {
    if (proxy.remoteMulticastLocator.kind ==
        LocatorKind_t::LOCATOR_KIND_INVALID) {
      proxy.suppressUnicast = false;
      proxy.useMulticast = false;
    } else {
      bool found = false;
      for (auto &avproxy : m_proxies) {
        if (avproxy.remoteMulticastLocator.kind ==
                LocatorKind_t::LOCATOR_KIND_UDPv4 &&
            avproxy.remoteMulticastLocator.getIp4Address().addr ==
                proxy.remoteMulticastLocator.getIp4Address().addr &&
            avproxy.remoteLocator.getIp4Address().addr !=
                proxy.remoteLocator.getIp4Address().addr) {
          if (avproxy.suppressUnicast == false) {
            avproxy.useMulticast = false;
            avproxy.suppressUnicast = true;
            proxy.useMulticast = true;
            proxy.suppressUnicast = true;
            SLW_LOG("Found Multicast Partner!\n");
            if (avproxy.remoteReaderGuid.entityId !=
                proxy.remoteReaderGuid.entityId) {
              proxy.unknown_eid = true;
              SLW_LOG("Found different EntityIds, using UNKNOWN_ENTITYID\n");
            }
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
void StatelessWriterT<NetworkDriver>::resetSendOptions() {
  for (auto &proxy : m_proxies) {
    proxy.suppressUnicast = false;
    proxy.useMulticast = false;
    proxy.unknown_eid = false;
  }
  manageSendOptions();
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::removeReader(const Guid_t &guid) {
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const ReaderProxy &proxy) {
    return proxy.remoteReaderGuid == guid;
  };
  auto thunk = [](void *arg, const ReaderProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
  resetSendOptions();
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::removeReaderOfParticipant(
    const GuidPrefix_t &guidPrefix) {
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const ReaderProxy &proxy) {
    return proxy.remoteReaderGuid.prefix == guidPrefix;
  };
  auto thunk = [](void *arg, const ReaderProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
  resetSendOptions();
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

  SLW_LOG("Adding new data.\n");
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

  if (m_proxies.getNumElements() == 0) {
    SLW_LOG("No Proxy!\n");
  }

  for (const auto &proxy : m_proxies) {

    SLW_LOG("Progess.\n");
    // Do nothing, if someone else sends for me... (Multicast)
    if (proxy.useMulticast || !proxy.suppressUnicast || m_enforceUnicast) {
      PacketInfo info;
      info.srcPort = m_packetInfo.srcPort;

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

  ++m_nextSequenceNumberToSend;
}

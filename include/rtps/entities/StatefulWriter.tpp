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

#include "rtps/entities/StatefulWriter.h"

#include "rtps/messages/MessageFactory.h"
#include <cstring>
#include <stdio.h>

using rtps::StatefulWriterT;

/*
static int line_cnt_ = 0;
static char tft_buffer_[150];

#define log(args...) if(true){ \
Lock lock{m_mutex}; \
snprintf(tft_buffer_, sizeof(tft_buffer_), args); 		 \
TFT_PrintLine(line_cnt_, tft_buffer_); 					 \
line_cnt_ = (line_cnt_+1)%5; \
}
*/
#define SFW_VERBOSE 0

#if SFW_VERBOSE
#include "rtps/utils/printutils.h"
#endif

template <class NetworkDriver>
StatefulWriterT<NetworkDriver>::~StatefulWriterT() {
  m_running = false;
  sys_msleep(10); // Required for tests/ Join currently not available
                  // if(sys_mutex_valid(&m_mutex)){
  sys_mutex_free(&m_mutex);
  //}
}

template <class NetworkDriver>
bool StatefulWriterT<NetworkDriver>::init(TopicData attributes,
                                          TopicKind_t topicKind,
                                          ThreadPool * /*threadPool*/,
                                          NetworkDriver &driver) {
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
#if SFW_VERBOSE
    log("StatefulWriter: Failed to create mutex.\n");
#endif
    return false;
  }

  m_transport = &driver;
  m_attributes = attributes;
  m_topicKind = topicKind;
  m_packetInfo.srcPort = attributes.unicastLocator.port;
  if (m_attributes.endpointGuid.entityId ==
      ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER) {
    m_heartbeatThread = sys_thread_new("HBThreadPub", hbFunctionJumppad, this,
                                       Config::HEARTBEAT_STACKSIZE,
                                       Config::THREAD_POOL_WRITER_PRIO);
  } else if (m_attributes.endpointGuid.entityId ==
             ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER) {
    m_heartbeatThread = sys_thread_new("HBThreadSub", hbFunctionJumppad, this,
                                       Config::HEARTBEAT_STACKSIZE,
                                       Config::THREAD_POOL_WRITER_PRIO);
  } else {
    m_heartbeatThread = sys_thread_new("HBThread", hbFunctionJumppad, this,
                                       Config::HEARTBEAT_STACKSIZE,
                                       Config::THREAD_POOL_WRITER_PRIO);
  }

  m_is_initialized_ = true;
  return true;
}

template <class NetworkDriver>
bool StatefulWriterT<NetworkDriver>::addNewMatchedReader(
    const ReaderProxy &newProxy) {
#if SFW_VERBOSE
  log("StatefulWriter[%s]: New reader added with id: ",
      &this->m_attributes.topicName[0]);
  printGuid(newProxy.remoteReaderGuid);
  log("\n");
#endif
  bool success = m_proxies.add(newProxy);
  manageSendOptions();
  return success;
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::manageSendOptions() {
#if SFW_VERBOSE
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
#if SFW_VERBOSE
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
void StatefulWriterT<NetworkDriver>::removeReader(const Guid &guid) {
  auto isElementToRemove = [&](const ReaderProxy &proxy) {
    return proxy.remoteReaderGuid == guid;
  };
  auto thunk = [](void *arg, const ReaderProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
const rtps::CacheChange *StatefulWriterT<NetworkDriver>::newChange(
    ChangeKind_t kind, const uint8_t *data, DataSize_t size) {
  if (isIrrelevant(kind)) {
    return nullptr;
  }

  Lock lock{m_mutex};

  if (m_history.isFull()) {
    // Right now we drop elements anyway because we cannot detect non-responding
    // readers yet. return nullptr;
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

#if SFW_VERBOSE
  log("StatefulWriter[%s]: Adding new data.\n", this->m_attributes.topicName);
#endif
  return result;
}

template <class NetworkDriver> void StatefulWriterT<NetworkDriver>::progress() {
  for (const auto &proxy : m_proxies) {
    if (!sendDataWRMulticast(proxy, m_nextSequenceNumberToSend)) {
      continue;
    }
  }
  ++m_nextSequenceNumberToSend;
}

template <typename NetworkDriver>
bool StatefulWriterT<NetworkDriver>::isIrrelevant(ChangeKind_t kind) const {
  // Right now we only allow alive changes
  // return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY
  // && kind != ChangeKind_t::ALIVE);
  return kind != ChangeKind_t::ALIVE;
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::setAllChangesToUnsent() {
  Lock lock(m_mutex);

  m_nextSequenceNumberToSend = m_history.getSeqNumMin();

  if (mp_threadPool != nullptr) {
    mp_threadPool->addWorkload(this);
  }
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::onNewAckNack(
    const SubmessageAckNack &msg, const GuidPrefix_t &sourceGuidPrefix) {
  // Lock lock{m_mutex};
  // Search for reader
  ReaderProxy *reader = nullptr;
  for (auto &proxy : m_proxies) {
    if (proxy.remoteReaderGuid.prefix == sourceGuidPrefix &&
        proxy.remoteReaderGuid.entityId == msg.readerId) {
      reader = &proxy;
      break;
    }
  }

  if (reader == nullptr) {
#if SFW_VERBOSE
    log("StatefulWriter[%s]: No proxy found with id: ",
        &this->m_attributes.topicName[0]);
    printEntityId(msg.readerId);
    log(" Dropping acknack.\n");
#endif
    return;
  }

  uint8_t hash = 0;
  for (int i = 0; i < sourceGuidPrefix.id.size(); i++) {
    hash += sourceGuidPrefix.id.at(i);
  }

  char bfr[20];
  size_t size = snprintf(bfr, sizeof(bfr), "%u <= %u", msg.count.value,
                         reader->ackNackCount.value);
  if (!(size < sizeof(bfr))) {
    while (1)
      ;
  }

  if (msg.count.value <= reader->ackNackCount.value) {
#if SFW_VERBOSE
    log("StatefulWriter[%s]: Count too small. Dropping acknack.\n",
        &this->m_attributes.topicName[0]);
#endif
    return;
  }

  reader->ackNackCount = msg.count;

  // Send missing packets
  SequenceNumber_t nextSN = msg.readerSNState.base;
#if SFW_VERBOSE
  if (nextSN.low == 0 && nextSN.high == 0) {
    log("StatefulWriter[%s]: Received preemptive acknack. Ignored.\n",
        &this->m_attributes.topicName[0]);
  } else {
    log("StatefulWriter[%s]: Received non-preemptive acknack.\n",
        &this->m_attributes.topicName[0]);
  }
#endif
  for (uint32_t i = 0; i < msg.readerSNState.numBits; ++i, ++nextSN) {
    if (msg.readerSNState.isSet(i)) {
#if SFW_VERBOSE
      log("StatefulWriter[%s]: Send Packet on acknack.\n",
          this->m_attributes.topicName);
#endif
      sendData(*reader, nextSN);
    }
  }
  // Check for sequence numbers after defined range
  SequenceNumber_t maxSN;
  {
    Lock lock(m_mutex);
    maxSN = m_history.getSeqNumMax();
  }
  while (nextSN <= maxSN) {
    sendData(*reader, nextSN);
    ++nextSN;
  }
}

template <class NetworkDriver>
bool StatefulWriterT<NetworkDriver>::sendData(
    const ReaderProxy &reader, const SequenceNumber_t &snMissing) {

  // TODO smarter packaging e.g. by creating MessageStruct and serialize after
  // adjusting values Reusing the pbuf is not possible. See
  // https://www.nongnu.org/lwip/2_0_x/raw_api.html (Zero-Copy MACs)

  PacketInfo info;
  info.srcPort = m_packetInfo.srcPort;

  MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
  MessageFactory::addSubMessageTimeStamp(info.buffer);

  // Just usable for IPv4
  const Locator &locator = reader.remoteLocator;

  info.destAddr = locator.getIp4Address();
  info.destPort = (Ip4Port_t)locator.port;

  {
    Lock lock(m_mutex);
    const CacheChange *next = m_history.getChangeBySN(snMissing);
    if (next == nullptr) {
#if SFW_VERBOSE
      log("StatefulWriter[%s]: Couldn't get a CacheChange with SN (%i,%u)\n",
          &this->m_attributes.topicName[0], snMissing.high, snMissing.low);
#endif
      return false;
    }
    MessageFactory::addSubMessageData(
        info.buffer, next->data, false, next->sequenceNumber,
        m_attributes.endpointGuid.entityId, reader.remoteReaderGuid.entityId);
  }

  m_transport->sendPacket(info);
  return true;
}

template <class NetworkDriver>
bool StatefulWriterT<NetworkDriver>::sendDataWRMulticast(
    const ReaderProxy &reader, const SequenceNumber_t &snMissing) {

  // TODO smarter packaging e.g. by creating MessageStruct and serialize after
  // adjusting values Reusing the pbuf is not possible. See
  // https://www.nongnu.org/lwip/2_0_x/raw_api.html (Zero-Copy MACs)
  if(reader.useMulticast || reader.suppressUnicast == false) {
    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;

    MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    MessageFactory::addSubMessageTimeStamp(info.buffer);

    // Just usable for IPv4
    // Deceide whether multicast or not
    if(reader.useMulticast) {
      const Locator &locator = reader.remoteMulticastLocator;
      info.destAddr = locator.getIp4Address();
      info.destPort = (Ip4Port_t)locator.port;
    } else {
      const Locator &locator = reader.remoteLocator;
      info.destAddr = locator.getIp4Address();
      info.destPort = (Ip4Port_t)locator.port;
    }

    {
      Lock lock(m_mutex);
      const CacheChange *next = m_history.getChangeBySN(snMissing);
      if (next == nullptr) {
  #if SFW_VERBOSE
        log("StatefulWriter[%s]: Couldn't get a CacheChange with SN (%i,%u)\n",
            &this->m_attributes.topicName[0], snMissing.high, snMissing.low);
  #endif
        return false;
      }

      EntityId_t reid;
      if(reader.useMulticast) {
        reid = ENTITYID_UNKNOWN;
      } else {
        reid = reader.remoteReaderGuid.entityId;
      }

      MessageFactory::addSubMessageData(
          info.buffer, next->data, false, next->sequenceNumber,
          m_attributes.endpointGuid.entityId, reid);
    }

    m_transport->sendPacket(info);
  }
  return true;
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::hbFunctionJumppad(void *thisPointer) {
  auto *writer = static_cast<StatefulWriterT<NetworkDriver> *>(thisPointer);
  writer->sendHeartBeatLoop();
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::sendHeartBeatLoop() {
  while (m_running) {
    sendHeartBeat();
    sys_msleep(Config::SF_WRITER_HB_PERIOD_MS);
  }
}

template <class NetworkDriver>
void StatefulWriterT<NetworkDriver>::sendHeartBeat() {
  if (m_proxies.isEmpty()) {
#if SFW_VERBOSE
    log("StatefulWriter[%s]: Skipping heartbeat. No proxies.\n",
        this->m_attributes.topicName);
#endif
    return;
  }

  for (auto &proxy : m_proxies) {

    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;

    SequenceNumber_t firstSN;
    SequenceNumber_t lastSN;
    MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    {
      Lock lock(m_mutex);
      firstSN = m_history.getSeqNumMin();
      lastSN = m_history.getSeqNumMax();
    }
    if (firstSN == SEQUENCENUMBER_UNKNOWN || lastSN == SEQUENCENUMBER_UNKNOWN) {
#if SFW_VERBOSE
      if (strlen(&this->m_attributes.typeName[0]) != 0) {
        log("StatefulWriter[%s]: Skipping heartbeat. No data.\n",
            this->m_attributes.topicName);
      }
#endif
      return;
    }

    MessageFactory::addHeartbeat(
        info.buffer, m_attributes.endpointGuid.entityId,
        proxy.remoteReaderGuid.entityId, firstSN, lastSN, m_hbCount);

    info.destAddr = proxy.remoteLocator.getIp4Address();
    info.destPort = proxy.remoteLocator.port;

    m_transport->sendPacket(info);
  }
  m_hbCount.value++;
}

#undef SFW_VERBOSE

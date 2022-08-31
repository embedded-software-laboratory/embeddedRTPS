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

#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "rtps/entities/StatefulReader.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/utils/Lock.h"

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define SFR_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatefulReader %s] ", &m_attributes.topicName[0]);                \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define SFR_LOG(...) //
#endif

using rtps::StatefulReaderT;

template <class NetworkDriver>
StatefulReaderT<NetworkDriver>::~StatefulReaderT() {}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::init(const TopicData &attributes,
                                          NetworkDriver &driver) {
  if (!initMutex()) {
    return false;
  }

  m_proxies.clear();
  m_attributes = attributes;
  m_transport = &driver;
  m_srcPort = attributes.unicastLocator.port;
  m_is_initialized_ = true;
  return true;
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::newChange(
    const ReaderCacheChange &cacheChange) {
  if (m_callback_count == 0 || !m_is_initialized_) {
    return;
  }
  Lock{m_proxies_mutex};
  for (auto &proxy : m_proxies) {
    if (proxy.remoteWriterGuid == cacheChange.writerGuid) {
      if (proxy.expectedSN == cacheChange.sn) {
        executeCallbacks(cacheChange);
        ++proxy.expectedSN;
        return;
      }
    }
  }
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::addNewMatchedWriter(
    const WriterProxy &newProxy) {
#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
  SFR_LOG("New writer added with id: ");
  printGuid(newProxy.remoteWriterGuid);
  SFR_LOG("\n");
#endif
  return m_proxies.add(newProxy);
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::onNewGapMessage(
    const SubmessageGap &msg, const GuidPrefix_t &remotePrefix) {
  Lock lock(m_proxies_mutex);
  if (!m_is_initialized_) {
    return false;
  }

  Guid_t writerProxyGuid;
  writerProxyGuid.prefix = remotePrefix;
  writerProxyGuid.entityId = msg.writerId;
  WriterProxy *writer = getProxy(writerProxyGuid);

  if (writer == nullptr) {

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
    SFR_LOG("Ignore GAP. Couldn't find a matching "
            "writer with id:");
    printEntityId(msg.writerId);
    SFR_LOG("\n");
#endif
    return false;
  }

  // We have not seen all messages leading up to gap start -> do nothing
  if (writer->expectedSN < msg.gapStart) {
    printf("GAP: Ignoring Gap, we have not seen all messages prior to gap "
           "begin: %u < %u\n",
           int(writer->expectedSN.low), int(msg.gapStart.low));
    return true;
  }

  // Start from base and search for first unset bit
  SequenceNumber_t first_valid = msg.gapList.base;
  for (unsigned int i = 0; i < msg.gapList.numBits; i++, first_valid++) {
    if (!msg.gapList.isSet(i)) {
      break;
    }
  }

  if (first_valid < writer->expectedSN) {
    SFR_LOG("GAP: Ignoring gap, we expect a message beyond the gap");
    return true;
  }

  SFR_LOG("GAP: moving expected SN to %u\n", (int)first_valid.low);
  writer->expectedSN = first_valid;

  // Send an ack nack message
  PacketInfo info;
  info.srcPort = m_srcPort;
  info.destAddr = writer->remoteLocator.getIp4Address();
  info.destPort = writer->remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  SequenceNumberSet set;
  set.numBits = 1;
  set.base = writer->expectedSN;
  set.bitMap[0] = uint32_t{1} << 31;
  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId, set,
                                   writer->getNextAckNackCount(), false);
  m_transport->sendPacket(info);

  return false;
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::onNewHeartbeat(
    const SubmessageHeartbeat &msg, const GuidPrefix_t &sourceGuidPrefix) {
  Lock lock(m_proxies_mutex);
  if (!m_is_initialized_) {
    return false;
  }
  PacketInfo info;
  info.srcPort = m_srcPort;

  Guid_t writerProxyGuid;
  writerProxyGuid.prefix = sourceGuidPrefix;
  writerProxyGuid.entityId = msg.writerId;
  WriterProxy *writer = getProxy(writerProxyGuid);

  if (writer == nullptr) {

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
    SFR_LOG("Ignore heartbeat. Couldn't find a matching "
            "writer with id:");
    printEntityId(msg.writerId);
    SFR_LOG("\n");
#endif
    return false;
  }

  if (msg.count.value <= writer->hbCount.value) {

    SFR_LOG("Ignore heartbeat. Count too low.\n");
    return false;
  }

  writer->hbCount.value = msg.count.value;
  info.destAddr = writer->remoteLocator.getIp4Address();
  info.destPort = writer->remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
                                   writer->getMissing(msg.firstSN, msg.lastSN),
                                   writer->getNextAckNackCount(), false);

  SFR_LOG("Sending acknack.\n");
  m_transport->sendPacket(info);
  return true;
}

#undef SFR_VERBOSE

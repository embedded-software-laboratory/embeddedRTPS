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
#include "rtps/utils/Diagnostics.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define SFR_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatefulReader %s] ", &m_attributes.topicName[0]);                \
    printf(__VA_ARGS__);                                                       \
    printf("\r\n");                                                            \
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
  Lock lock{m_proxies_mutex};
  for (auto &proxy : m_proxies) {
    if (proxy.remoteWriterGuid == cacheChange.writerGuid) {
      if (proxy.expectedSN == cacheChange.sn) {
        SFR_LOG("Delivering SN %u.%u | ! GUID %u %u %u %u \r\n",
                (int)cacheChange.sn.high, (int)cacheChange.sn.low,
                cacheChange.writerGuid.prefix.id[0],
                cacheChange.writerGuid.prefix.id[1],
                cacheChange.writerGuid.prefix.id[2],
                cacheChange.writerGuid.prefix.id[3]);
        executeCallbacks(cacheChange);
        ++proxy.expectedSN;
        SFR_LOG("Done processing SN %u.%u\r\n", (int)cacheChange.sn.high,
               (int)cacheChange.sn.low);
        return;
      } else {
        Diagnostics::StatefulReader::sfr_unexpected_sn++;
        SFR_LOG(
            "Unexpected SN %u.%u != %u.%u, dropping! GUID %u %u %u %u | \r\n",
            (int)proxy.expectedSN.high, (int)proxy.expectedSN.low,
            (int)cacheChange.sn.high, (int)cacheChange.sn.low,
            cacheChange.writerGuid.prefix.id[0],
            cacheChange.writerGuid.prefix.id[1],
            cacheChange.writerGuid.prefix.id[2],
            cacheChange.writerGuid.prefix.id[3]);
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
  Lock lock{m_proxies_mutex};
  if (!m_is_initialized_) {
    return false;
  }
  SFR_LOG("Processing gap message %u %u", msg.gapStart, msg.gapList.base);

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

  // Case 1: We are still waiting for messages before gapStart
  if (writer->expectedSN < msg.gapStart) {
    PacketInfo info;
    info.srcPort = m_srcPort;
    info.destAddr = writer->remoteLocator.getIp4Address();
    info.destPort = writer->remoteLocator.port;
    rtps::MessageFactory::addHeader(info.buffer,
                                    m_attributes.endpointGuid.prefix);
    SequenceNumber_t last_valid = msg.gapStart;
    --last_valid;
    auto missing_sns = writer->getMissing(writer->expectedSN, last_valid);
    rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
                                     missing_sns, writer->getNextAckNackCount(),
                                     false);
    m_transport->sendPacket(info);
    return true;
  }

  // Case 2: We are expecting a message between [gapStart; gapList.base -1]
  // Advance expectedSN beyond gapList.base
  if (writer->expectedSN < msg.gapList.base) {
    auto before = writer->expectedSN;
    writer->expectedSN = msg.gapList.base;

    // writer->expectedSN++;

    // Advance expectedSN to first unset bit
    for (uint32_t bit = 0; bit < SNS_MAX_NUM_BITS;
         writer->expectedSN++, bit++) {
      if (!msg.gapList.isSet(bit)) {
        break;
      }
    }

    return true;

  }else{

	  // Case 3: We are expecting a sequence number beyond gap list base,
	  // check if we need to update expectedSN
	  auto i = msg.gapList.base;
	  for(uint32_t bit = 0; bit < SNS_MAX_NUM_BITS; i++, bit++){
		if(i < writer->expectedSN){
			continue;
		}

		if(msg.gapList.isSet(bit)){
			writer->expectedSN++;
		}else{
		  PacketInfo info;
		  info.srcPort = m_srcPort;
		  info.destAddr = writer->remoteLocator.getIp4Address();
		  info.destPort = writer->remoteLocator.port;
		  rtps::MessageFactory::addHeader(info.buffer,
											m_attributes.endpointGuid.prefix);
		  SequenceNumberSet set;
		  set.base = writer->expectedSN;
		  set.numBits = 1;
		  set.bitMap[0] = set.bitMap[0] |= uint32_t{1} << 31;
		  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
											 set, writer->getNextAckNackCount(),
											 false);
		  m_transport->sendPacket(info);

		  return true;
		}
	  }


    return false;
  }
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::onNewHeartbeat(
    const SubmessageHeartbeat &msg, const GuidPrefix_t &sourceGuidPrefix) {
  Lock lock{m_proxies_mutex};
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

  if (writer->expectedSN < msg.firstSN) {
    SFR_LOG("expectedSN < firstSN, advancing expectedSN");
    writer->expectedSN = msg.firstSN;
  }

  writer->hbCount.value = msg.count.value;
  info.destAddr = writer->remoteLocator.getIp4Address();
  info.destPort = writer->remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  auto missing_sns = writer->getMissing(msg.firstSN, msg.lastSN);
  bool final_flag = (missing_sns.numBits == 0);
  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
                                   missing_sns, writer->getNextAckNackCount(),
                                   final_flag);

  SFR_LOG("Sending acknack base %u bits %u .\n", (int)missing_sns.base.low,
          (int)missing_sns.numBits);
  m_transport->sendPacket(info);
  return true;
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::sendPreemptiveAckNack(
    const WriterProxy &writer) {
  Lock lock{m_proxies_mutex};
  if (!m_is_initialized_) {
    return false;
  }

  PacketInfo info;
  info.srcPort = m_attributes.unicastLocator.port;
  info.destAddr = writer.remoteLocator.getIp4Address();
  info.destPort = writer.remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  SequenceNumberSet number_set;
  number_set.base.high = 0;
  number_set.base.low = 0;
  number_set.numBits = 0;
  rtps::MessageFactory::addAckNack(
      info.buffer, writer.remoteWriterGuid.entityId,
      m_attributes.endpointGuid.entityId, number_set, Count_t{1}, false);

  SFR_LOG("Sending preemptive acknack.\n");
  m_transport->sendPacket(info);
  return true;
}

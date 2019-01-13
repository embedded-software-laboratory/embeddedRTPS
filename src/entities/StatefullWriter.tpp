/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullWriter.h"

#include "rtps/messages/MessageFactory.h"
#include <cstring>

using rtps::StatefullWriterT;

#define SFW_VERBOSE 1

template <class NetworkDriver>
bool StatefullWriterT<NetworkDriver>::init(BuiltInTopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, UdpDriver& driver){
    if (sys_mutex_new(&m_mutex) != ERR_OK) {
        printf("StatefullWriter: Failed to create mutex.\n");
        return false;
    }

    mp_threadPool = threadPool;
    m_transport = &driver;
    m_attributes = attributes;
    m_topicKind = topicKind;
    m_packetInfo.srcPort = attributes.unicastLocator.port;

    m_heartbeatThread = sys_thread_new("HeartbeatThread", hbFunctionJumppad, this, Config::THREAD_POOL_WRITER_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);

    return true;
}

template <class NetworkDriver>
bool StatefullWriterT<NetworkDriver>::addNewMatchedReader(const ReaderProxy& newProxy){
#if SFW_VERBOSE
    printf("StatefullWriter[%s]: New reader added.\n", &this->m_attributes.topicName[0]);
#endif
    for(uint32_t i=0; i < sizeof(m_proxies)/sizeof(m_proxies[0]); ++i){
        static_assert(sizeof(i)*8 >= sizeof(m_proxySlotUsedBitMap), "StatelessWriter: Loop variable too small");

        if((m_proxySlotUsedBitMap & (1 << i)) == 0){
            m_proxies[i] = newProxy;
            m_proxySlotUsedBitMap |= (1 << i);
            return true;
        }
    }

    return false;
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::progress(){

}

template <class NetworkDriver>
const rtps::CacheChange* StatefullWriterT<NetworkDriver>::newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size){
    Lock lock{m_mutex};
    if(m_history.isFull()){
        return nullptr;
    }

    ++m_lastChangeSequenceNumber;
    CacheChange change{};
    change.kind = kind;
    change.data.reserve(size);
    change.data.append(data, size);
    change.sequenceNumber = m_lastChangeSequenceNumber;

#if SFW_VERBOSE
    printf("StatefullWriter[%s]: Adding new data.\n", this->m_attributes.topicName);
#endif
    return m_history.addChange(std::move(change));
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::unsentChangesReset(){
    // Don't see a reason why this might be needed for a reliable writer
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::onNewAckNack(const SubmessageAckNack& msg){

    // Search for reader
    ReaderProxy* proxy = nullptr;
    for(uint8_t i=0; i < sizeof(m_proxies)/sizeof(m_proxies[0]); ++i){
        if(((m_proxySlotUsedBitMap & (1 << i)) != 0) && m_proxies[i].remoteReaderGuid.entityId == msg.readerId){
            proxy = &m_proxies[i];
            break;
        }
    }

    if(proxy == nullptr || msg.count.value < proxy->ackNackCount.value){
#if SFW_VERBOSE
        printf("StatefullWriter[%s]: Dropping acknack.\n", &this->m_attributes.topicName[0]);
#endif
        return;
    }

    proxy->ackNackCount = msg.count;

    // Send missing packets
    SequenceNumber_t nextSN = msg.readerSNState.base;
#if SFW_VERBOSE
    if(nextSN.low == 0 && nextSN.high == 0){
        printf("StatefullWriter[%s]: Received preemptive acknack.\n", &this->m_attributes.topicName[0]);
    }else{
        printf("StatefullWriter[%s]: Received non-preemptive acknack.\n", &this->m_attributes.topicName[0]);
    }
#endif
    for(uint32_t i=0; i < msg.readerSNState.numBits; ++i, ++nextSN){
        if(msg.readerSNState.isSet(i)){
#if SFW_VERBOSE
            printf("StatefullWriter[%s]: Send Packet on acknack.\n", this->m_attributes.topicName);
#endif
            sendData(*proxy, nextSN);
        }
    }
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::sendData(const ReaderProxy &reader, const SequenceNumber_t &sn){
    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;

    MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    MessageFactory::addSubMessageTimeStamp(info.buffer);

    {
        Lock lock(m_mutex);
        const CacheChange* next = m_history.getChangeBySN(sn);
        if(next == nullptr){
#if SFW_VERBOSE
            printf("StatefullWriter[%s]: Couldn't get a CacheChange with SN (%i,%u)\n", &this->m_attributes.topicName[0], sn.high, sn.low);
#endif
            return;
        }
        MessageFactory::addSubMessageData(info.buffer, next->data, false, next->sequenceNumber, m_attributes.endpointGuid.entityId,
                                          reader.remoteReaderGuid.entityId);
    }

    // Just usable for IPv4
    const Locator& locator = reader.remoteLocator;

    info.destAddr = locator.getIp4Address();
    info.destPort = (Ip4Port_t) locator.port;

    m_transport->sendFunction(info);
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::hbFunctionJumppad(void* thisPointer){
    auto writer = static_cast<StatefullWriter*>(thisPointer);
    while(1){
        writer->sendHeartBeat();
        sys_msleep(m_heartbeatPeriodMs);
    }
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::sendHeartBeat() {
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
    if(firstSN == SEQUENCENUMBER_UNKNOWN || lastSN == SEQUENCENUMBER_UNKNOWN){
#if SFW_VERBOSE
        if(strlen(&this->m_attributes.typeName[0]) != 0){
            printf("StatefullWriter[%s]: Skipping heartbeat. No data.\n", this->m_attributes.topicName);
        }
#endif
        return;
    }

    MessageFactory::addHeartbeat(info.buffer, m_attributes.endpointGuid.entityId, ENTITYID_UNKNOWN, firstSN, lastSN, m_hbCount);

    // Just usable for IPv4
    const Locator& locator = getBuiltInMulticastLocator();

    info.destAddr = locator.getIp4Address();
    info.destPort = (Ip4Port_t) locator.port;

    m_transport->sendFunction(info);
}

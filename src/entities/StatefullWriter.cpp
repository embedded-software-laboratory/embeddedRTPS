/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullWriter.h"

#include "rtps/messages/MessageFactory.h"

using rtps::StatefullWriter;

bool StatefullWriter::init(BuiltInTopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, UdpDriver& driver){
    if (sys_mutex_new(&m_mutex) != ERR_OK) {
        printf("Failed to create mutex \n");
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

bool StatefullWriter::addNewMatchedReader(const ReaderProxy& newProxy){
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

void StatefullWriter::progress(){

}

const rtps::CacheChange* StatefullWriter::newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size){
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

    return m_history.addChange(std::move(change));
}

void StatefullWriter::unsentChangesReset(){
    // Don't see a reason why this might be needed for a reliable writer
}

void StatefullWriter::onNewAckNack(const SubmessageAckNack& msg){
    // Search for reader
    ReaderProxy* proxy = nullptr;
    for(uint8_t i=0; i < sizeof(m_proxies)/sizeof(m_proxies[0]); ++i){
        if(((m_proxySlotUsedBitMap & (1 << i)) != 0) && m_proxies[i].remoteReaderGuid.entityId == msg.readerId){
            proxy = &m_proxies[i];
            break;
        }
    }

    if(proxy == nullptr || msg.count.value < proxy->ackNackCount.value){
        return;
    }

    proxy->ackNackCount = msg.count;

    // Send missing packets
    SequenceNumber_t nextSN = msg.readerSNState.base;
    for(uint32_t i=0; i < msg.readerSNState.numBits; ++i, ++nextSN){
        if(msg.readerSNState.isSet(i)){
            printf("Send Packet on acknack\n");
            sendData(*proxy, nextSN);
        }
    }
}

void StatefullWriter::sendData(const ReaderProxy &reader, const SequenceNumber_t &sn){
    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;

    MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    MessageFactory::addSubMessageTimeStamp(info.buffer);

    {
        Lock lock(m_mutex);
        const CacheChange* next = m_history.getChangeBySN(sn);
        if(next == nullptr){
            printf("StatelessWriter: Couldn't get a CacheChange with SN (%i,%u)\n", sn.high, sn.low);
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

void StatefullWriter::hbFunctionJumppad(void* thisPointer){
    auto writer = static_cast<StatefullWriter*>(thisPointer);
    while(1){
        writer->sendHeartBeat();
        sys_msleep(m_heartbeatPeriodMs);
    }
}

void StatefullWriter::sendHeartBeat() {
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
        return;
    }

    MessageFactory::addHeartbeat(info.buffer, m_attributes.endpointGuid.entityId, ENTITYID_UNKNOWN, firstSN, lastSN, m_hbCount);

    // Just usable for IPv4
    const Locator& locator = getBuiltInMulticastLocator();

    info.destAddr = locator.getIp4Address();
    info.destPort = (Ip4Port_t) locator.port;

    m_transport->sendFunction(info);
}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullWriter.h"

using rtps::StatefullWriter;

bool StatefullWriter::init(TopicKind_t topicKind, ThreadPool* threadPool,
                           GuidPrefix_t guidPrefix, EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort){
    if (sys_mutex_new(&m_mutex) != ERR_OK) {
        printf("Failed to create mutex \n");
        return false;
    }

    mp_threadPool = threadPool;
    m_transport = &driver;
    m_guid.prefix = guidPrefix;
    m_guid.entityId = entityId;
    m_topicKind = topicKind;
    m_packetInfo.srcPort = sendPort;

    return true;
}

bool StatefullWriter::addNewMatchedReader(const ReaderProxy& newProxy){
    for(uint32_t i=0; i < sizeof(m_proxies)/sizeof(m_proxies[0]); ++i){
        static_assert(sizeof(i)*8 >= sizeof(m_proxySlotUsedBitMap), "StatelessWriter: Loop variable too small");

        if((m_proxySlotUsedBitMap & (1 << i)) == 0){
            m_proxies[i] = newProxy;
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

    auto result = m_history.addChange(std::move(change));

    return result;
}

void StatefullWriter::unsentChangesReset(){

}
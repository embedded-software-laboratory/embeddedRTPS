/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessWriter.h"

#include "rtps/ThreadPool.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/udpUtils.h"

namespace rtps{

    void StatelessWriter::init(TopicKind_t topicKind, ThreadPool* threadPool,
                          GuidPrefix_t guidPrefix, EntityId_t entityId, Ip4Port_t sendPort){
        mp_threadPool = threadPool;
        m_guidPrefix = guidPrefix;
        m_entityId = entityId;
        m_topicKind = topicKind;
        m_sendPort = sendPort;
        if (sys_mutex_new(&m_mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
        }
    }

    bool StatelessWriter::addNewMatchedReader(ReaderLocator loc){
        if(m_readerLocator.entityId != ENTITYID_UNKNOWN){
            return false;
        }
        m_readerLocator = loc;
        return true;
    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return m_lastChangeSequenceNumber;
    }

    const CacheChange* StatelessWriter::newChange(rtps::ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
        if(isIrrelevant(kind)){
            return &m_history.INVALID_CACHE_CHANGE;
        }

        ++m_lastChangeSequenceNumber;
        CacheChange change{};
        change.kind = kind;
        change.data.reserve(size);
        change.data.append(data, size);
        change.sequenceNumber = m_lastChangeSequenceNumber;


        Lock lock(m_mutex);
        auto result = m_history.addChange(std::move(change));
        if(mp_threadPool != nullptr){
            mp_threadPool->addWorkload(ThreadPool::Workload_t{this, 1});
        }
        return result;
    }

    void StatelessWriter::unsentChangesReset() {
        Lock lock(m_mutex);
        auto numReset = m_history.resetSend();
        if(mp_threadPool != nullptr){
            mp_threadPool->addWorkload(ThreadPool::Workload_t{this, numReset});
        }
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    bool StatelessWriter::createMessageCallback(ThreadPool::PacketInfo& packetInfo){
        if(m_readerLocator.entityId == ENTITYID_UNKNOWN){ // TODO UNKNOWN might be okay. Detect not-set locator in another way
            return false;
        }

        MessageFactory::addHeader(packetInfo.buffer, m_guidPrefix);
        MessageFactory::addSubMessageTimeStamp(packetInfo.buffer);

        {
            Lock lock(m_mutex);
            const CacheChange* next = m_history.getNextCacheChange();
            MessageFactory::addSubMessageData(packetInfo.buffer, next->data, false, next->sequenceNumber, m_entityId,
                                              m_readerLocator.entityId); // TODO
        }

        // Just usable for IPv4
        const Locator& locator = m_readerLocator.locator;

        packetInfo.srcPort = m_sendPort;
        IP4_ADDR((&packetInfo.destAddr), locator.address[12], locator.address[13], locator.address[14], locator.address[15]);
        packetInfo.destPort = (Ip4Port_t) locator.port;
        return true;
    }
}
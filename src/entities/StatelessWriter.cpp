/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageFactory.h"
#include "rtps/entities/StatelessWriter.h"

namespace rtps{

    void StatelessWriter::init(TopicKind_t topicKind, Locator_t locator, ThreadPool* threadPool,
                          GuidPrefix_t guidPrefix, EntityId_t entityId){
        m_threadPool = threadPool;
        m_guidPrefix = guidPrefix;
        m_entityId = entityId;
        m_topicKind = topicKind;
        m_locator = locator;
        if (sys_mutex_new(&mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
        }
    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return m_lastChangeSequenceNumber;
    }

    const CacheChange* StatelessWriter::newChange(rtps::ChangeKind_t kind, const uint8_t* data, data_size_t size) {
        if(isIrrelevant(kind)){
            return &m_history.INVALID_CACHE_CHANGE;
        }

        ++m_lastChangeSequenceNumber;
        CacheChange change{};
        change.kind = kind;
        change.data.reserve(size);
        change.data.append(data, size);
        change.sequenceNumber = m_lastChangeSequenceNumber;


        Lock lock(mutex);
        auto result = m_history.addChange(std::move(change));
        if(m_threadPool != nullptr){
            m_threadPool->addWorkload(ThreadPool::Workload_t{this, 1});
        }
        return result;
    }

    void StatelessWriter::unsentChangesReset() {
        Lock lock(mutex);
        auto numReset = m_history.resetSend();
        if(m_threadPool != nullptr){
            m_threadPool->addWorkload(ThreadPool::Workload_t{this, numReset});
        }
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::createMessageCallback(PBufWrapper& buffer){
        MessageFactory::addHeader(buffer, m_guidPrefix);
        MessageFactory::addSubMessageTimeStamp(buffer);

        {
            Lock lock(mutex);
            const CacheChange* next = m_history.getNextCacheChange();
            MessageFactory::addSubMessageData(buffer, next->data, false, next->sequenceNumber, m_entityId);
        }

        // Just usable for IPv4
        IP4_ADDR((&buffer.addr), m_locator.address[12],m_locator.address[13],m_locator.address[14], m_locator.address[15]);
        buffer.port = (ip4_port_t) m_locator.port;
    }



}
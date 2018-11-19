/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageFactory.h"
#include "rtps/entities/StatelessWriter.h"

namespace rtps{

    StatelessWriter::StatelessWriter(TopicKind_t topicKind, Locator_t locator, ThreadPool* threadPool,
                                     GuidPrefix_t guidPrefix, EntityId_t entityId)
        : threadPool(threadPool), guidPrefix(guidPrefix), entityId(entityId), topicKind(topicKind), locator(locator){
        if (sys_mutex_new(&mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
        }
    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return lastChangeSequenceNumber;
    }

    const CacheChange* StatelessWriter::newChange(rtps::ChangeKind_t kind, const uint8_t* data, data_size_t size) {
        if(isIrrelevant(kind)){
            return &history.INVALID_CACHE_CHANGE;
        }

        ++lastChangeSequenceNumber;
        CacheChange change{};
        change.kind = kind;
        change.data.reserve(size);
        change.data.append(data, size);
        change.sequenceNumber = lastChangeSequenceNumber;


        Lock lock(mutex);
        auto result = history.addChange(std::move(change));
        if(threadPool != nullptr){
            threadPool->addWorkload(ThreadPool::Workload_t{this, 1});
        }
        return result;
    }

    void StatelessWriter::unsentChangesReset() {
        Lock lock(mutex);
        auto numReset = history.resetSend();
        if(threadPool != nullptr){
            threadPool->addWorkload(ThreadPool::Workload_t{this, numReset});
        }
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::createMessageCallback(PBufWrapper& buffer){
        Lock lock(mutex);

        const CacheChange* next = history.getNextCacheChange();

        MessageFactory::addHeader(buffer, guidPrefix);
        MessageFactory::addSubMessageTimeStamp(buffer);
        MessageFactory::addSubMessageData(buffer, next->data, false, next->sequenceNumber, entityId);

        // Just usable for IPv4
        IP4_ADDR((&buffer.addr), locator.address[12],locator.address[13],locator.address[14], locator.address[15]);
        buffer.port = (ip4_port_t) locator.port;
    }



}
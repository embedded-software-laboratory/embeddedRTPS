/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageFactory.h"
#include "rtps/entities/StatelessWriter.h"

namespace rtps{

    StatelessWriter::StatelessWriter(TopicKind_t topicKind): topicKind(topicKind){
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
        return history.addChange(std::move(change));
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::createMessageCallback(PBufWrapper& buffer){
        Lock lock(mutex);

        const CacheChange* next = history.getNextCacheChange();

        MessageFactory::addHeader(buffer, GUIDPREFIX_UNKNOWN);
        MessageFactory::addSubMessageTimeStamp(buffer);
        MessageFactory::addSubMessageData(buffer, next->data, false);

        IP4_ADDR((&buffer.addr), 192,168,0, 42);
        buffer.port = 7050;
    }



}
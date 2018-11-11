/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageFactory.h"
#include "rtps/entities/StatelessWriter.h"

namespace rtps{

    StatelessWriter::StatelessWriter(TopicKind_t topicKind): topicKind(topicKind){

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

        return history.addChange(std::move(change));
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::createMessageCallback(PBufWrapper& buffer){
        //const CacheChange* next = history.getNextCacheChange();

        //pbuf* originalFirst = next->data.firstElement;
        //pbuf* originalLast = next->data.
        //MessageFactory::addHeader(buffer, GUIDPREFIX_UNKNOWN);
        //MessageFactory::addSubMessageData(buffer, next->data, PBufWrapper{});
    }



}
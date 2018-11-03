/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessWriter.h"

namespace rtps{

    StatelessWriter::StatelessWriter(TopicKind_t topicKind): topicKind(topicKind){

    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return lastChangeSequenceNumber;
    }

    CacheChange StatelessWriter::newChange(rtps::ChangeKind_t kind, uint8_t* data, data_size_t size) {
        if(isIrrelevant(kind)){
            return CacheChange{};
        }

        ++lastChangeSequenceNumber;
        change.kind = kind;
        change.data = data;
        change.size = size;
        return change;
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

}
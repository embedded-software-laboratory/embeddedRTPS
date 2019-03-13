/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef PROJECT_CACHECHANGE_H
#define PROJECT_CACHECHANGE_H

#include "rtps/common/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{
    struct CacheChange{
        ChangeKind_t kind = ChangeKind_t::INVALID;
        SequenceNumber_t sequenceNumber = SEQUENCENUMBER_UNKNOWN;
        PBufWrapper data{};

        CacheChange() = default;
        CacheChange(ChangeKind_t kind, SequenceNumber_t sequenceNumber)
                : kind(kind), sequenceNumber(sequenceNumber){};
    };
}

#endif //PROJECT_CACHECHANGE_H

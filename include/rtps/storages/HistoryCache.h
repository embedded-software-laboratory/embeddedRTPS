/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_HISTORYCACHE_H
#define RTPS_HISTORYCACHE_H

#include "rtps/types.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/ThreadSafeCircularBuffer.h"

namespace rtps{

    struct CacheChange{
        ChangeKind_t kind = ChangeKind_t::INVALID;
        SequenceNumber_t sequenceNumber = SEQUENCENUMBER_UNKNOWN;
        PBufWrapper data{};

        CacheChange() = default;
        CacheChange(ChangeKind_t kind, SequenceNumber_t sequenceNumber)
            : kind(kind), sequenceNumber(sequenceNumber){};
    };

    class HistoryCache{
    public:
        const CacheChange INVALID_CACHE_CHANGE{};

        const CacheChange* addChange(CacheChange&& change);
        const CacheChange* getNextCacheChange();

        const SequenceNumber_t& getSeqNumMin() const;
        const SequenceNumber_t& getSeqNumMax() const;

        /**
         * @return Number of changes that were already sent
         */
        uint8_t resetSend();

    private:
        struct HistoryEntry{
            bool send = false;
            CacheChange change;
        };

        std::array<HistoryEntry, 10> buffer{};
        uint16_t head = 0;
        uint16_t tail = 0;
        uint16_t lastReturned = 0;

        inline void incrementHead();
        inline void incrementIterator(uint16_t& iterator) const;
        inline void incrementTail();
    };
};

#endif //RTPS_HISTORYCACHE_H

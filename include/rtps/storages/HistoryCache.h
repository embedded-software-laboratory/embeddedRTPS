/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_HISTORYCACHE_H
#define RTPS_HISTORYCACHE_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/CacheChange.h"

namespace rtps{

    /**
     * This class can be used when we need to invalidate data in between or the sequence numbers
     * are not consecutive
     */
    class HistoryCache{
    public:
        /**
         * Adds a new Change. If the buffer is full, it will override
         * the oldest one. If you want to avoid this, use isFull() and dropFirst().
         */
        const CacheChange* addChange(CacheChange&& newChange);
        void dropFirst();
        bool isFull() const;

        const CacheChange* getChangeBySN(const SequenceNumber_t& sn) const;

        const SequenceNumber_t& getSeqNumMin() const;
        const SequenceNumber_t& getSeqNumMax() const;

    private:

        std::array<CacheChange, Config::HISTORY_SIZE + 1> m_buffer{};
        uint16_t m_head = 0;
        uint16_t m_tail = 0;
        static_assert(sizeof(rtps::Config::HISTORY_SIZE) < sizeof(m_head), "Iterator is large enough for given size");

        inline void incrementHead();
        inline void incrementIterator(uint16_t& iterator) const;
        inline void incrementTail();
    };
}

#endif //RTPS_HISTORYCACHE_H

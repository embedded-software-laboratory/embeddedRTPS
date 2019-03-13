/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef PROJECT_SIMPLEHISTORYCACHE_H
#define PROJECT_SIMPLEHISTORYCACHE_H

#include "rtps/storages/CacheChange.h"
#include "rtps/config.h"

namespace rtps{

    /**
     * Simple version of a hitory cache. It sets consecutive sequence numbers automatically which
     * allows an easy and fast approach of dropping acknowledged changes. Furthermore, disposing
     * of arbitrary changes is not possible. However, this is in principle easy to add by changing
     * the ChangeKind and dropping it when passing it during deleting of other sequence numbers
     */
    class SimpleHistoryCache{
    public:
        SimpleHistoryCache() = default;

        bool isFull() const;
        const CacheChange* addChange(const uint8_t* data, DataSize_t size);
        void dropOldest();
        void removeUntilIncl(SequenceNumber_t sn);
        const CacheChange* getChangeBySN(SequenceNumber_t sn) const;

        const SequenceNumber_t& getSeqNumMin() const;
        const SequenceNumber_t& getSeqNumMax() const;

    private:
        std::array<CacheChange, Config::HISTORY_SIZE + 1> m_buffer{};
        uint16_t m_head = 0;
        uint16_t m_tail = 0;
        static_assert(sizeof(rtps::Config::HISTORY_SIZE) < sizeof(m_head), "Iterator is large enough for given size");

        SequenceNumber_t m_lastUsedSequenceNumber{0,0};

        inline void incrementHead();
        inline void incrementIterator(uint16_t& iterator) const;
        inline void incrementTail();
    protected:
        // This constructor was created for unit testing
        explicit SimpleHistoryCache(SequenceNumber_t SN);
    };
}

#endif //PROJECT_SIMPLEHISTORYCACHE_H

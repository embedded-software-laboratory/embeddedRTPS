/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;

const rtps::CacheChange* HistoryCache::addChange(CacheChange&& change){
    auto& entry = m_buffer[m_head];
    entry.change = change;
    entry.send = false;
    entry.change = std::move(change);

    incrementHead();

    return &entry.change;
}

bool HistoryCache::isFull() const{
    auto iterator = m_head;
    incrementIterator(iterator);
    return iterator == m_tail;
}

uint8_t HistoryCache::resetSend() {
    uint8_t numReset = 0;
    auto iterator = m_tail;
    while(iterator != m_head){
        auto& entry = m_buffer[iterator];
        if(entry.send){
            ++numReset;
            entry.send = false;
        }
        incrementIterator(iterator);
    }
    m_lastReturned = m_tail;
    return numReset;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMin() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;
    auto iterator = m_tail;
    while(iterator != m_head){
        auto& entry = m_buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || entry.change.sequenceNumber < *pSN) {
            pSN = &entry.change.sequenceNumber;
        }
    incrementIterator(iterator);
    }

    return *pSN;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMax() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;

    auto iterator = m_tail;
    while(iterator != m_head){
        auto& entry = m_buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || *pSN < entry.change.sequenceNumber){
            pSN = &entry.change.sequenceNumber;
        }
        incrementIterator(iterator);
    }
    return *pSN;
}

const rtps::CacheChange* HistoryCache::getNextCacheChange(){
    if(m_lastReturned != m_head){
        auto& entry = m_buffer[m_lastReturned];
        entry.send = true;
        incrementIterator(m_lastReturned);
        return &entry.change;
    }else{
        return &INVALID_CACHE_CHANGE;
    }
}

void HistoryCache::incrementHead() {
    incrementIterator(m_head);
    if(m_head == m_tail){
        incrementTail();
    }
}

void HistoryCache::incrementTail() {
    if(m_lastReturned == m_tail){
        incrementIterator(m_lastReturned);
    }
    incrementIterator(m_tail);
}

void HistoryCache::incrementIterator(uint16_t& iterator) const{
    ++iterator;
    if(iterator >= m_buffer.size()){
        iterator = 0;
    }
}

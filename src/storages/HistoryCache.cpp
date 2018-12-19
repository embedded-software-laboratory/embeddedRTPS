/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;

const rtps::CacheChange* HistoryCache::addChange(CacheChange&& newChange){
    auto& change = m_buffer[m_head];
    change = std::move(newChange);

    incrementHead();

    return &change;
}

void HistoryCache::dropFirst() {
    incrementTail();
}

bool HistoryCache::isFull() const{
    auto iterator = m_head;
    incrementIterator(iterator);
    return iterator == m_tail;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMin() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;
    auto iterator = m_tail;
    while(iterator != m_head){
        auto& change = m_buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || change.sequenceNumber < *pSN) {
            pSN = &change.sequenceNumber;
        }
    incrementIterator(iterator);
    }

    return *pSN;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMax() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;

    auto iterator = m_tail;
    while(iterator != m_head){
        auto& change = m_buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || *pSN < change.sequenceNumber){
            pSN = &change.sequenceNumber;
        }
        incrementIterator(iterator);
    }
    return *pSN;
}

const rtps::CacheChange* HistoryCache::getChangeBySN(const SequenceNumber_t& sn) const{
    auto iterator = m_tail;
    while(iterator != m_head){
        auto& change = m_buffer[iterator];
        if(change.sequenceNumber == sn){
            return &change;
        }
        incrementIterator(iterator);
    }

    return nullptr;
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

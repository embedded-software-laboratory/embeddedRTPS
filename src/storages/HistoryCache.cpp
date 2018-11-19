/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;

HistoryCache::HistoryCache() {
    err_t err = sys_mutex_new(&mutex);
    if(err != ERR_OK){
        printf("HistoryCache: Unable to create mutex.");
    }
}
const rtps::CacheChange* HistoryCache::addChange(CacheChange&& change){
    Lock lock(mutex);
    auto& entry = buffer[head];
    entry.change = change;
    entry.send = false;
    entry.change = std::move(change);

    incrementHead();

    return &entry.change;
}

uint8_t HistoryCache::resetSend() {
    Lock lock(mutex);
    uint8_t numReset = 0;
    auto iterator = tail;
    while(iterator != head){
        auto& entry = buffer[iterator];
        if(entry.send){
            ++numReset;
            entry.send = false;
        }
        incrementIterator(iterator);
        lastReturned = tail;
    }
    return numReset;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMin() const{
    Lock lock(mutex);
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;
    auto iterator = tail;
    while(iterator != head){
        auto& entry = buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || entry.change.sequenceNumber < *pSN) {
            pSN = &entry.change.sequenceNumber;
        }
    incrementIterator(iterator);
    }

    return *pSN;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMax() const{
    Lock lock(mutex);
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;

    auto iterator = tail;
    while(iterator != head){
        auto& entry = buffer[iterator];
        if(pSN == &SEQUENCENUMBER_UNKNOWN || *pSN < entry.change.sequenceNumber){
            pSN = &entry.change.sequenceNumber;
        }
        incrementIterator(iterator);
    }
    return *pSN;
}

const rtps::CacheChange* HistoryCache::getNextCacheChange(){
    Lock lock(mutex);
    if(lastReturned != head){
        auto& entry = buffer[lastReturned];
        entry.send = true;
        incrementIterator(lastReturned);
        return &entry.change;
    }else{
        return &INVALID_CACHE_CHANGE;
    }
}

void HistoryCache::incrementHead() {
    incrementIterator(head);
    if(head == tail){
        incrementTail();
    }
}

void HistoryCache::incrementTail() {
    if(lastReturned == tail){
        incrementIterator(lastReturned);
    }
    incrementIterator(tail);
}

void HistoryCache::incrementIterator(uint16_t& iterator) const{
    ++iterator;
    if(iterator >= buffer.size()){
        iterator = 0;
    }
}

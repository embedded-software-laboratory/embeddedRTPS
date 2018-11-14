/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;

const rtps::CacheChange* HistoryCache::addChange(CacheChange&& change){
    for(auto& entry : buffer){
        if(!entry.used){
            entry.used = true;
            entry.send = false;
            entry.change = std::move(change);
            return &entry.change;
        }
    }
    return &INVALID_CACHE_CHANGE;
}

void HistoryCache::removeChange(const CacheChange* change){
    for(auto& entry : buffer){
        if(change == &entry.change){
            entry.used = false;
            return;
        }
    }
}

uint8_t HistoryCache::resetSend() {
    uint8_t numReset = 0;
    for(auto& entry : buffer){
        if(entry.used && entry.send){
            ++numReset;
            entry.send = false;
        }
    }
    return numReset;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMin() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;
    for(auto const& entry : buffer){
        if(entry.used){
            if(pSN == &SEQUENCENUMBER_UNKNOWN || entry.change.sequenceNumber < *pSN) {
                pSN = &entry.change.sequenceNumber;
            }
        }
    }
    return *pSN;
}

const rtps::SequenceNumber_t& HistoryCache::getSeqNumMax() const{
    const SequenceNumber_t* pSN = &SEQUENCENUMBER_UNKNOWN;
    for(auto const& entry : buffer){
        if(entry.used){
            if(pSN == &SEQUENCENUMBER_UNKNOWN || *pSN < entry.change.sequenceNumber){
                pSN = &entry.change.sequenceNumber;
            }
        }
    }
    return *pSN;
}

const rtps::CacheChange* HistoryCache::getNextCacheChange(){
    for(size_t i=0; i < buffer.size(); ++i){
        if(lastReturned >= buffer.size() -1){
            lastReturned = 0;
        }else{
            lastReturned++;
        }
        if(buffer[lastReturned].used){
            buffer[lastReturned].send = true;
            return &(buffer[lastReturned].change);
        }
    }
    return &INVALID_CACHE_CHANGE;
}

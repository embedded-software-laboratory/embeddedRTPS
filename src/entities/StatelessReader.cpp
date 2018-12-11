/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"

using rtps::StatelessReader;

void StatelessReader::newChange(ReaderCacheChange& cacheChange){
    if(m_callback != nullptr){
        m_callback(m_callee, cacheChange);
    }
}

void StatelessReader::registerCallback(ddsReaderCallback_fp cb, void* callee){
    if(cb != nullptr && callee != nullptr){
        m_callback = cb;
        m_callee = callee;
    }else{
        printf("StatelessReader: Callback or callee nullptr");
    }
}

void StatelessReader::onNewHeartbeat(const SubmessageHeartbeat&, const GuidPrefix_t&){
    // nothing to do
}

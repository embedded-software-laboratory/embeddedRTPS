/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"

using rtps::StatelessReader;

void StatelessReader::init(const BuiltInTopicData& attributes){
    m_attributes = attributes;
}

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

bool StatelessReader::addNewMatchedWriter(const WriterProxy& /*newProxy*/){
    // Nothing to do
    return true;
}

bool StatelessReader::onNewHeartbeat(const SubmessageHeartbeat&, const GuidPrefix_t&){
    // nothing to do
    return true;
}

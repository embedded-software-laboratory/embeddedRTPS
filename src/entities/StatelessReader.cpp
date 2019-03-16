/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"

using rtps::StatelessReader;

#define SLR_VERBOSE 0


void StatelessReader::init(const TopicData& attributes){
    m_attributes = attributes;
}

void StatelessReader::newChange(ReaderCacheChange& cacheChange){
    if(m_callback != nullptr){
        m_callback(m_callee, cacheChange);
    }
}

void StatelessReader::registerCallback(ddsReaderCallback_fp cb, void* callee){
    if(cb != nullptr){
        m_callback = cb;
        m_callee = callee; // It's okay if this is null
    }else{
#if SLR_VERBOSE
        printf("StatelessReader[%s]: Passed callback is nullptr\n", &m_attributes.topicName[0]);
#endif
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

#undef SLR_VERBOSE

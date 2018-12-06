/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"

using rtps::StatelessReader;

void StatelessReader::newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size){
    if(m_callback != nullptr){
        m_callback(m_callee, kind, data, size);
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

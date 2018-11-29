/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessReader.h"



using rtps::StatelessReader;


void StatelessReader::newChange(ChangeKind_t /*kind*/, const uint8_t* data, data_size_t size){
    if(m_callback != nullptr){
        m_callback(data, size);
    }
}

void StatelessReader::registerCallback(ddsReaderCallback_fp cb){
    m_callback = cb;
}

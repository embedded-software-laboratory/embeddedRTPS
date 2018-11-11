/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CUSTOMMATCHER_H
#define RTPS_CUSTOMMATCHER_H

#include <gmock/gmock.h>
#include  "rtps/storages/PBufWrapper.h"

MATCHER_P2(PBufContains, data, size, ""){
    if(size==0){
        return true;
    }
    int i=0;
    pbuf* current = arg.firstElement;
    while(current != nullptr){
        auto pBufData = static_cast<uint8_t*>(current->payload);
        for(auto offset=0; offset < current->len; ++offset){
            if(data[i] != pBufData[offset]){
                return false;
            }

            i++;

            if(size <= i){
                return true;
            }
        }
        current = current->next;
    }
    return false;
}

#endif //RTPS_CUSTOMMATCHER_H

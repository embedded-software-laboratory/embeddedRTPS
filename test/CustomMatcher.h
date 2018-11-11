/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CUSTOMMATCHER_H
#define RTPS_CUSTOMMATCHER_H

#include <gmock/gmock.h>
#include  "rtps/storages/PBufWrapper.h"

MATCHER_P2(PBufContains, data, size, ""){
    pbuf* current = arg.firstElement;

    return (pbuf_memcmp(current, 0, data, size) == 0);

}

#endif //RTPS_CUSTOMMATCHER_H

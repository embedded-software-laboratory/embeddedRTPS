/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CUSTOMMATCHER_H
#define RTPS_CUSTOMMATCHER_H

#include <gmock/gmock.h>
#include  "rtps/storages/PBufWrapper.h"


MATCHER_P(PBufContains, data, ""){
    pbuf* current = arg.firstElement;

    return current != nullptr && (pbuf_memcmp(current, 0, data.data(), data.size()) == 0);

}

#endif //RTPS_CUSTOMMATCHER_H

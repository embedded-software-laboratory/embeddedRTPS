/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPS_H
#define RTPS_RTPS_H

#include "rtps/common/types.h"
#include "rtps/ThreadPool.h"

namespace rtps{
    void init();
    Time_t getCurrentTimeStamp();
}

#endif //RTPS_RTPS_H

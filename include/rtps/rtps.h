/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPS_H
#define RTPS_RTPS_H

#include "types.h"
#include "rtps/ThreadPool.h"

namespace rtps{
    static ThreadPool threadPool;
    void init();
    void start();
    void stop();
    Time_t getCurrentTimeStamp();
}

#endif //RTPS_RTPS_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPS_H
#define RTPS_RTPS_H

#include "rtps/common/types.h"

namespace rtps{

#if defined(unix) || defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    void init();
#endif
    Time_t getCurrentTimeStamp();
}

#endif //RTPS_RTPS_H

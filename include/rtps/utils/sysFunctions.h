/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef PROJECT_SYSFUNCTIONS_H
#define PROJECT_SYSFUNCTIONS_H

#include "rtps/common/types.h"
#include "lwip/sys.h"

namespace rtps{
    inline Time_t getCurrentTimeStamp(){
        Time_t now;
        // TODO FIX
        uint32_t nowMs = sys_now();
        now.seconds = (int32_t) nowMs / 1000;
        now.fraction = ((nowMs % 1000)/1000);
        return now;
    }
}


#endif //PROJECT_SYSFUNCTIONS_H

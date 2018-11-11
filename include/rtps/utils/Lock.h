/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_LOCK_H
#define RTPS_LOCK_H

#include "lwip/sys.h"

namespace rtps{
    class Lock {
    public:
        explicit Lock(sys_mutex_t &passedMutex) : mutex(passedMutex) {
            sys_mutex_lock(&mutex);
        };

        ~Lock() {
            sys_mutex_unlock(&mutex);
        };
    private:
        sys_mutex_t &mutex;
    };
}
#endif //RTPS_LOCK_H

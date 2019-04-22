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
        explicit Lock(sys_mutex_t& passedMutex) : m_mutex(passedMutex) {
            sys_mutex_lock(&m_mutex);
        };

        ~Lock() {
            sys_mutex_unlock(&m_mutex);
        };
    private:
        sys_mutex_t& m_mutex;
    };
}
#endif //RTPS_LOCK_H

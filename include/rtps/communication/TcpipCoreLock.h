/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_TCPIPCORELOCK_H
#define RTPS_TCPIPCORELOCK_H

#include <lwip/tcpip.h>

namespace rtps{
        class TcpipCoreLock{
            public:
            TcpipCoreLock(){
                LOCK_TCPIP_CORE();
            }
            ~TcpipCoreLock(){
                UNLOCK_TCPIP_CORE();
            }
        };
}

#endif //RTPS_TCPIPCORELOCK_H

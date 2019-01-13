/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPINTERFACE_H
#define RTPS_UDPINTERFACE_H

#include "lwip/udp.h"
#include "rtps/communication/TcpipCoreLock.h"

namespace rtps{
    class LwipInterface{
    public:
        static udp_pcb *udpNew(void){
            TcpipCoreLock lock;
            return udp_new();
        }

        static void udpRemove(udp_pcb *pcb){
            TcpipCoreLock lock;
            udp_remove(pcb);
        }
    };
}
#endif //RTPS_UDPINTERFACE_H

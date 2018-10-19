/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPINTERFACE_H
#define RTPS_UDPINTERFACE_H

#include "lwip/udp.h"
class LwipInterface{
public:
    static udp_pcb *udpNew(void){
        return udp_new();
    }

    static void udpRemove(udp_pcb *pcb){
        udp_remove(pcb);
    }
};
#endif //RTPS_UDPINTERFACE_H

/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_LWIPINTERFACEMOCK_H
#define RTPS_LWIPINTERFACEMOCK_H

#include "lwip/udp.h"
#include <iostream>
#include "gmock/gmock.h"

class LwipInterfaceMock {
public:
    static udp_pcb *udpNew(void) {
        static udp_pcb *id = new udp_pcb;
        printf("Creating pcb  pcb %p", id);
        id++;
        return id;
    }

    static void udpRemove(udp_pcb *pcb) {
        printf("Removing pcb %p", pcb);
        delete pcb;
    }
};
#endif //RTPS_LWIPINTERFACEMOCK_H

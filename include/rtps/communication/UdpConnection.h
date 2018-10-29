/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPCONNECTION_H
#define RTPS_UDPCONNECTION_H

#include "lwip/udp.h"
#include "LwipInterface.h"

template <class UdpInterface = LwipInterface>
struct UdpConnectionT{
    udp_pcb* pcb = nullptr;
    ip4_addr addr;
    uint16_t port;
    bool is_multicast = false;

    UdpConnectionT() = default;

    UdpConnectionT(ip4_addr addr, uint16_t port)
    : addr(addr), port(port){
        pcb = UdpInterface::udpNew();
    }

    UdpConnectionT& operator=(UdpConnectionT&& other){
        addr = other.addr;
        port = other.port;
        is_multicast = other.is_multicast;

        if(other.pcb != nullptr){
            if(pcb != nullptr){
                UdpInterface::udpRemove(pcb);
            }
            pcb = other.pcb;
            other.pcb = nullptr;
        }
        return *this;
    }

    ~UdpConnectionT(){
        if(pcb != nullptr){
            UdpInterface::udpRemove(pcb);
            pcb = nullptr;
        }
    }

};

using UdpConnection = UdpConnectionT<>; // <> required before C17
#endif //RTPS_UDPCONNECTION_H

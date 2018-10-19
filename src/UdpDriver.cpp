/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/UdpDriver.h"

#include <algorithm>


bool UdpDriver::createUdpConnection(const ip4_addr addr, const uint16_t port) {

    for(auto const &conn : conns){
        if(conn.addr.addr == addr.addr && conn.port == port){
            return true;
        }
    }

    if(n_conns == conns.size()){
        return false;
    }
    LOCK_TCPIP_CORE();
    UdpConnection udp_conn(addr, port);
    err_t err = udp_bind(udp_conn.pcb, IP_ADDR_ANY, port); //to receive multicast
    UNLOCK_TCPIP_CORE();
    if(err != ERR_OK && err != ERR_USE){
        printf("Failed to bind to %s:%u: error %u", ipaddr_ntoa(&addr), port, err);
        return false;
    }

    conns[n_conns] = std::move(udp_conn);
    n_conns++;

    printf("Success creating UDP connection");
    return true;
}

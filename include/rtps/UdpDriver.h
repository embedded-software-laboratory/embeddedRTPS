/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPDRIVER_H
#define RTPS_UDPDRIVER_H

#include "lwip/udp.h"
#include "rtps/config.h"
#include "rtps/UdpConnection.h"
#include "rtps/LwipInterface.h"
#include "rtps/types.h"

#include <array>

class UdpDriver{
public:
    typedef void (*udp_rx_func_t)(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, ip4_port_t port);


    constexpr ip4_addr transforIP4ToU32(uint8_t MSB, uint8_t p2, uint8_t p1, uint8_t LSB){
        return {((uint32_t)(MSB << 24)) |
                ((uint32_t)( p2 << 16)) |
                ((uint32_t)( p1 <<  8)) |
                            LSB};
    }
    
    constexpr ip4_addr defaultMCastGroup(){
        return transforIP4ToU32(239,255,0,1);
    }


    bool createUdpConnection(const ip4_addr addr, const ip4_port_t port, const udp_rx_func_t callback);
    bool sendPacket(const ip4_addr destAddr, const ip4_port_t destPort, const uint8_t *data, size_t length);
private:
    std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> conns;
    std::size_t n_conns = 0;

};

#endif //RTPS_UDPDRIVER_H

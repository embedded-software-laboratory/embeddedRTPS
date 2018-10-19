/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPDRIVER_H
#define RTPS_UDPDRIVER_H

#include "lwip/udp.h"
#include "rtps/config.h"
#include "rtps/UdpConnection.h"
#include "rtps/LwipInterface.h"

#include <array>



class UdpDriver{
public:
    bool createUdpConnection(const ip4_addr addr, const uint16_t port);
    template<std::size_t SIZE>
    bool sendPacket(const ip4_addr destAddr, const uint16_t destPort, const std::array<uint8_t, SIZE> &data) const{
        auto begin = conns.cbegin();
        auto end = conns.cend();
        while(begin != end){
            if(begin->addr.addr == destAddr.addr && begin->port == destPort){
                break;
            }
            begin++;
        }

        if(begin == end){
            printf("Could not find a udp connection for destination %s:%u,...adding\r\n", ipaddr_ntoa(&destAddr), destPort);
            return false;
        }

        const UdpConnection& conn = *begin;

        pbuf* pb = pbuf_alloc(PBUF_TRANSPORT, SIZE, PBUF_POOL);
        if (!pb) {
            printf("Error while allocating pbuf\n\r");
            return false;
        }

        memcpy(pb->payload, data.data(), SIZE);
        LOCK_TCPIP_CORE();
        err_t err = udp_sendto(conn.pcb, pb, &(destAddr), destPort);
        UNLOCK_TCPIP_CORE();
        if(err != ERR_OK){
            printf("UDP TRANSMIT NOT SUCCESSFULL %s:%u size: %u err: %i\r\n", ipaddr_ntoa(&destAddr), destPort, SIZE, err);
            pbuf_free(pb);
            return false;
        }
        pbuf_free(pb);
        printf("Send packet successful \n");
        return true;
    }
private:
    std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> conns;
    std::size_t n_conns = 0;

};


#endif //RTPS_UDPDRIVER_H

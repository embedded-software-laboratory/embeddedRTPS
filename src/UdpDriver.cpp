/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/UdpDriver.h"



/**
 *
 * @param addr IP4 address on which we are listening
 * @param port Port on which we are listening
 * @param callback Function that gets called when a packet is received on addr:port.
 * @return True if creation was finished without errors. False otherwise.
 */
bool UdpDriver::createUdpConnection(const ip4_addr addr, const ip4_port_t port, const udp_rx_func_t callback) {

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
    LOCK_TCPIP_CORE();
    udp_recv(udp_conn.pcb, callback, nullptr);
    UNLOCK_TCPIP_CORE();

    conns[n_conns] = std::move(udp_conn);
    n_conns++;

    printf("Success creating UDP connection\n");
    return true;
}

bool UdpDriver::sendPacket(const ip4_addr destAddr, const ip4_port_t destPort, const uint8_t *data, size_t length){
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
        createUdpConnection(destAddr, destPort, nullptr);
        begin = conns.begin() + std::distance(conns.data(), &conns[n_conns]);
    }

    const UdpConnection& conn = *begin;

    pbuf* pb = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_POOL);
    if (!pb) {
        printf("Error while allocating pbuf\n\r");
        return false;
    }

    memcpy(pb->payload, data, length);
    LOCK_TCPIP_CORE();
    err_t err = udp_sendto(conn.pcb, pb, &(destAddr), destPort);
    UNLOCK_TCPIP_CORE();
    if(err != ERR_OK){
        printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\r\n", ipaddr_ntoa(&destAddr), destPort, length, err);
        pbuf_free(pb);
        return false;
    }
    pbuf_free(pb);
    printf("Send packet successful \n");
    return true;
}
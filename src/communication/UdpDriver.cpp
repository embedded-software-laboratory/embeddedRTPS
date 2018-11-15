/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/communication/UdpDriver.h"
#include "rtps/storages/PBufWrapper.h"
#include "lwip/tcpip.h"


using rtps::UdpDriver;
    /**
 *
 * @param addr IP4 address on which we are listening
 * @param port Port on which we are listening
 * @param callback Function that gets called when a packet is received on addr:port.
 * @return True if creation was finished without errors. False otherwise.
 */
const rtps::UdpConnection* UdpDriver::createUdpConnection(const ip4_addr_t &addr, ip4_port_t port, udp_rx_func_t callback, void* args) {

    for(auto const &conn : m_conns){
        if(conn.addr.addr == addr.addr && conn.port == port){
            return &conn;
        }
    }

    if(m_numConns == m_conns.size()){
        return nullptr;
    }

    UdpConnection udp_conn(addr, port);
    //LOCK_TCPIP_CORE();
    err_t err = udp_bind(udp_conn.pcb, IP_ADDR_ANY, port); //to receive multicast
    //UNLOCK_TCPIP_CORE();

    if(err != ERR_OK && err != ERR_USE){
        printf("Failed to bind to %s:%u: error %u\n", ipaddr_ntoa(&addr), port, err);
        return nullptr;
    }
    //LOCK_TCPIP_CORE();
    udp_recv(udp_conn.pcb, callback, args);
    //UNLOCK_TCPIP_CORE();

    m_conns[m_numConns] = std::move(udp_conn);
    m_numConns++;

    printf("Success creating UDP connection\n");
    return &m_conns[m_numConns-1];
}

bool UdpDriver::sendPacket(const UdpConnection& conn, pbuf& buffer){

    err_t err = udp_sendto(conn.pcb, &buffer, &conn.addr, conn.port);

    if(err != ERR_OK){
        printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n", ipaddr_ntoa(&conn.addr), conn.port, buffer.tot_len, err);
        return false;
    }
    printf("Send packet successful \n");
    return true;
}
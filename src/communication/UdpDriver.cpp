/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/communication/UdpDriver.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/communication/TcpipCoreLock.h"

#include "rtps/storages/PBufWrapper.h"

#include <lwip/igmp.h>
#include <lwip/tcpip.h>


using rtps::UdpDriver;

#define UDP_DRIVER_VERBOSE 0

UdpDriver::UdpDriver(rtps::UdpDriver::udpRxFunc_fp callback, void *args)
    : m_rxCallback(callback), m_callbackArgs(args){

}


/**
 *
 * @param addr IP4 address on which we are listening
 * @param port Port on which we are listening
 * @param callback Function that gets called when a packet is received on addr:port.
 * @return True if creation was finished without errors. False otherwise.
 */
const rtps::UdpConnection* UdpDriver::createUdpConnection(Ip4Port_t receivePort) {
    for(uint8_t i=0; i < m_numConns; ++i){
        if(m_conns[i].port == receivePort){
            return &m_conns[i];
        }
    }

    if(m_numConns == m_conns.size()){
        return nullptr;
    }

    UdpConnection udp_conn(receivePort);

    {
        TcpipCoreLock lock;
        err_t err = udp_bind(udp_conn.pcb, IP_ADDR_ANY, receivePort); //to receive multicast

        if(err != ERR_OK && err != ERR_USE){
            // TODO printf("Failed to bind to port%u: error %u\n", receivePort, err);
            return nullptr;
        }

        udp_recv(udp_conn.pcb, m_rxCallback, m_callbackArgs);
    }


    m_conns[m_numConns] = std::move(udp_conn);
    m_numConns++;
#if UDP_DRIVER_VERBOSE
    printf("Successfully created UDP connection on port %u \n", receivePort);
#endif
    return &m_conns[m_numConns-1];
}

bool UdpDriver::joinMultiCastGroup(ip4_addr_t addr) const {
    err_t iret;

    {
        TcpipCoreLock lock;
        iret = igmp_joingroup(IP_ADDR_ANY, (&addr));
    }

    if(iret != ERR_OK){
#if UDP_DRIVER_VERBOSE
        printf("Failed to join IGMP multicast group %s\n", ipaddr_ntoa(&addr));
#endif
        return false;
    }else{
#if UDP_DRIVER_VERBOSE
        printf("Succesfully joined  IGMP multicast group %s\n", ipaddr_ntoa(&addr));
#endif
    }
    return true;
}

bool UdpDriver::sendPacket(const UdpConnection& conn, ip4_addr_t& destAddr, Ip4Port_t destPort, pbuf& buffer){
    err_t err;
    {
        TcpipCoreLock lock;
        err = udp_sendto(conn.pcb, &buffer, &destAddr, destPort);
    }

    if(err != ERR_OK){;
#if UDP_DRIVER_VERBOSE
        printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n", ipaddr_ntoa(&destAddr), destPort, buffer.tot_len, err);
#endif
        return false;
    }
    return true;
}

void UdpDriver::sendFunction(PacketInfo& packet){
    auto p_conn = createUdpConnection(packet.srcPort);
    if(p_conn == nullptr){;
#if UDP_DRIVER_VERBOSE
        printf("Failed to create connection on port %u \n", packet.srcPort);
#endif
        return;
    }

    sendPacket(*p_conn, packet.destAddr, packet.destPort, *packet.buffer.firstElement);
}

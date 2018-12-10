/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/communication/UdpDriver.h"
#include "rtps/storages/PBufWrapper.h"
#include <lwip/igmp.h>
#include <lwip/tcpip.h>


using rtps::UdpDriver;



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

    for(auto const &conn : m_conns){
        if(conn.port == receivePort){
            return &conn;
        }
    }

    if(m_numConns == m_conns.size()){
        return nullptr;
    }

    UdpConnection udp_conn(receivePort);
    //LOCK_TCPIP_CORE();
    err_t err = udp_bind(udp_conn.pcb, IP_ADDR_ANY, receivePort); //to receive multicast
    //UNLOCK_TCPIP_CORE();

    if(err != ERR_OK && err != ERR_USE){
        printf("Failed to bind to port%u: error %u\n", receivePort, err);
        return nullptr;
    }
    //LOCK_TCPIP_CORE();
    udp_recv(udp_conn.pcb, m_rxCallback, m_callbackArgs);
    //UNLOCK_TCPIP_CORE();

    m_conns[m_numConns] = std::move(udp_conn);
    m_numConns++;

    printf("Success creating UDP connection on port %u \n", receivePort);
    return &m_conns[m_numConns-1];
}

bool UdpDriver::joinMultiCastGroup(ip4_addr_t addr) const {
    err_t iret = igmp_joingroup(IP_ADDR_ANY, (&addr));

    if(iret != ERR_OK){
        printf("Failed to join IGMP multicast group %s\n", ipaddr_ntoa(&addr));
        return false;
    }
    return true;
}

bool UdpDriver::sendPacket(const UdpConnection& conn, ip4_addr_t& destAddr, Ip4Port_t destPort, pbuf& buffer){

    err_t err = udp_sendto(conn.pcb, &buffer, &destAddr, destPort);

    if(err != ERR_OK){
        printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n", ipaddr_ntoa(&destAddr), destPort, buffer.tot_len, err);
        return false;
    }
    //printf("Send packet successful \n");
    return true;
}

void UdpDriver::sendFunction(PacketInfo& packet){

    auto p_conn = createUdpConnection(packet.srcPort);
    if(p_conn == nullptr){
        printf("Failed to create connection on port %u \n", packet.srcPort);
        return;
    }

    sendPacket(*p_conn, packet.destAddr, packet.destPort, *packet.buffer.firstElement);
}

/**
 * This function needs to be executed by the tcpip-thread
 */
void UdpDriver::sendFunctionJumppad(void* packetInfo){
    auto& packet = *static_cast<PacketInfo*>(packetInfo);
    packet.transport->sendFunction(packet);
}
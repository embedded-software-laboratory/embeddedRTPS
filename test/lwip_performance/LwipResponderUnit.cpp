/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include "test/lwip_performance/LwipResponderUnit.h"

#include "test/lwip_performance/LwipLatencyTopic.h"

#include "lwip/udp.h"
#include <iostream>

using rtps::tests::LwipResponderUnit;

LwipResponderUnit::LwipResponderUnit() : m_recvPort(7410){
    prepareLwip();
}

void LwipResponderUnit::prepareLwip(){
    LOCK_TCPIP_CORE();
    m_pcb = udp_new();
    err_t err = udp_bind(m_pcb, IP_ADDR_ANY, m_recvPort); //to receive multicast

    if(err != ERR_OK && err != ERR_USE){
        printf("Failed to bind to port%u: error %u\n", m_recvPort, err);
        return;
    }

    udp_recv(m_pcb, LwipResponderUnit::responderJumppad, this);
    printf("Successfully created UDP connection on port %u \n", m_recvPort);
    UNLOCK_TCPIP_CORE();
}

void LwipResponderUnit::responderJumppad(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port) {
    auto& responder = *static_cast<LwipResponderUnit*>(args);
    responder.responderCallback(args, target, pbuf, addr, port);
}

void LwipResponderUnit::responderCallback(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port){
    err_t err = udp_sendto(m_pcb, pbuf, addr, port);

    if(err != ERR_OK){
        printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n", ipaddr_ntoa(addr), port, pbuf->tot_len, err);
    }
}

void LwipResponderUnit::run(){
    while(true); // Just serve all the time
}

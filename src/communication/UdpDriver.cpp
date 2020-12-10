/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#include "rtps/communication/UdpDriver.h"
#include "rtps/communication/TcpipCoreLock.h"

#include <lwip/igmp.h>
#include <lwip/tcpip.h>

using rtps::UdpDriver;

#define UDP_DRIVER_VERBOSE 0

UdpDriver::UdpDriver(rtps::UdpDriver::udpRxFunc_fp callback, void *args)
    : m_rxCallback(callback), m_callbackArgs(args) {}

const rtps::UdpConnection *
UdpDriver::createUdpConnection(Ip4Port_t receivePort) {
  for (uint8_t i = 0; i < m_numConns; ++i) {
    if (m_conns[i].port == receivePort) {
      return &m_conns[i];
    }
  }

  if (m_numConns == m_conns.size()) {
    return nullptr;
  }

  UdpConnection udp_conn(receivePort);

  {
    TcpipCoreLock lock;
    err_t err = udp_bind(udp_conn.pcb, IP_ADDR_ANY,
                         receivePort); // to receive multicast

    if (err != ERR_OK && err != ERR_USE) {
      return nullptr;
    }

    udp_recv(udp_conn.pcb, m_rxCallback, m_callbackArgs);
  }

  m_conns[m_numConns] = std::move(udp_conn);
  m_numConns++;
#if UDP_DRIVER_VERBOSE
  printf("Successfully created UDP connection on port %u \n", receivePort);
#endif
  return &m_conns[m_numConns - 1];
}

bool UdpDriver::isSameSubnet(ip4_addr_t addr) {
  return (ip4_addr_netcmp(&addr, &(netif_default->ip_addr),
                          &(netif_default->netmask)) != 0);
}

bool UdpDriver::joinMultiCastGroup(ip4_addr_t addr) const {
  err_t iret;

  {
    TcpipCoreLock lock;
    iret = igmp_joingroup(IP_ADDR_ANY, (&addr));
  }

  if (iret != ERR_OK) {
#if UDP_DRIVER_VERBOSE
    printf("Failed to join IGMP multicast group %s\n", ipaddr_ntoa(&addr));
#endif
    return false;
  } else {
#if UDP_DRIVER_VERBOSE
    printf("Succesfully joined  IGMP multicast group %s\n", ipaddr_ntoa(&addr));
#endif
  }
  return true;
}

bool UdpDriver::sendPacket(const UdpConnection &conn, ip4_addr_t &destAddr,
                           Ip4Port_t destPort, pbuf &buffer) {
  err_t err;
  {
    TcpipCoreLock lock;
    err = udp_sendto(conn.pcb, &buffer, &destAddr, destPort);
  }

  if (err != ERR_OK) {
    ;
#if UDP_DRIVER_VERBOSE
    printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n",
           ipaddr_ntoa(&destAddr), destPort, buffer.tot_len, err);
#endif
    return false;
  }
  return true;
}

void UdpDriver::sendPacket(PacketInfo &packet) {
  auto p_conn = createUdpConnection(packet.srcPort);
  if (p_conn == nullptr) {
    ;
#if UDP_DRIVER_VERBOSE
    printf("Failed to create connection on port %u \n", packet.srcPort);
#endif
    return;
  }

  sendPacket(*p_conn, packet.destAddr, packet.destPort,
             *packet.buffer.firstElement);
}

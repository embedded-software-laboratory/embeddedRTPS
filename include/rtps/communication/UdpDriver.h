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

#ifndef RTPS_UDPDRIVER_H
#define RTPS_UDPDRIVER_H

#include "UdpConnection.h"
#include "lwip/udp.h"
#include "rtps/common/types.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/config.h"
#include "rtps/storages/PBufWrapper.h"

#include <array>

namespace rtps {

class UdpDriver {

public:
  typedef void (*udpRxFunc_fp)(void *arg, udp_pcb *pcb, pbuf *p,
                               const ip_addr_t *addr, Ip4Port_t port);

  UdpDriver(udpRxFunc_fp callback, void *args);

  const rtps::UdpConnection *createUdpConnection(Ip4Port_t receivePort);
  bool joinMultiCastGroup(ip4_addr_t addr) const;
  void sendPacket(PacketInfo &info);

  static bool isSameSubnet(ip4_addr_t addr);
  static bool isMulticastAddress(ip4_addr_t addr);

private:
  std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> m_conns;
  std::size_t m_numConns = 0;
  udpRxFunc_fp m_rxCallback = nullptr;
  void *m_callbackArgs = nullptr;

  bool sendPacket(const UdpConnection &conn, ip4_addr_t &destAddr,
                  Ip4Port_t destPort, pbuf &buffer);
};
} // namespace rtps

#endif // RTPS_UDPDRIVER_H

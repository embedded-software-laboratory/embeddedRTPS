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

#ifndef RTPS_UDPCONNECTION_H
#define RTPS_UDPCONNECTION_H

#include "TcpipCoreLock.h"
#include "lwip/udp.h"

namespace rtps {

struct UdpConnection {
  udp_pcb *pcb = nullptr;
  uint16_t port = 0;

  UdpConnection() = default; // Required for static allocation

  explicit UdpConnection(uint16_t port) : port(port) {
    LOCK_TCPIP_CORE();
    pcb = udp_new();
    UNLOCK_TCPIP_CORE();
  }

  UdpConnection &operator=(UdpConnection &&other) noexcept {
    port = other.port;

    if (pcb != nullptr) {
      LOCK_TCPIP_CORE();
      udp_remove(pcb);
      UNLOCK_TCPIP_CORE();
    }
    pcb = other.pcb;
    other.pcb = nullptr;
    return *this;
  }

  ~UdpConnection() {
    if (pcb != nullptr) {
      LOCK_TCPIP_CORE();
      udp_remove(pcb);
      UNLOCK_TCPIP_CORE();
      pcb = nullptr;
    }
  }
};
} // namespace rtps
#endif // RTPS_UDPCONNECTION_H

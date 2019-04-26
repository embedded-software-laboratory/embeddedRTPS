/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPCONNECTION_H
#define RTPS_UDPCONNECTION_H

#include "lwip/udp.h"
#include "TcpipCoreLock.h"

namespace rtps {

    struct UdpConnection {
        udp_pcb *pcb = nullptr;
        uint16_t port = 0;

        UdpConnection() = default; // Required for static allocation

        explicit UdpConnection(uint16_t port): port(port) {
            TcpipCoreLock lock;
            pcb =  udp_new();
        }

        UdpConnection& operator=(UdpConnection&& other) noexcept{
            port = other.port;

            if (pcb != nullptr) {
                TcpipCoreLock lock;
                udp_remove(pcb);
            }
            pcb = other.pcb;
            other.pcb = nullptr;
            return *this;
        }

        ~UdpConnection() {
            if (pcb != nullptr) {
                TcpipCoreLock lock;
                udp_remove(pcb);
                pcb = nullptr;
            }
        }

    };
}
#endif //RTPS_UDPCONNECTION_H

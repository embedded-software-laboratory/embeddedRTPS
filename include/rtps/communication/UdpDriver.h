/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDPDRIVER_H
#define RTPS_UDPDRIVER_H

#include "lwip/udp.h"
#include "rtps/config.h"
#include "UdpConnection.h"
#include "LwipInterface.h"
#include "rtps/types.h"

#include <array>

namespace rtps {

    class UdpDriver {
    private:
        std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> conns;
        std::size_t n_conns = 0;

    public:
        typedef void (*udp_rx_func_t)(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, ip4_port_t port);


        constexpr static ip4_addr transforIP4ToU32(uint8_t MSB, uint8_t p2, uint8_t p1, uint8_t LSB) {
            return {((uint32_t) (MSB << 24)) |
                    ((uint32_t) (p2 << 16)) |
                    ((uint32_t) (p1 << 8)) |
                    LSB};
        }

        constexpr static ip4_addr defaultMCastGroup() {
            return transforIP4ToU32(239, 255, 0, 1);
        }


        bool createUdpConnection(const ip4_addr_t &addr, const ip4_port_t port, const udp_rx_func_t callback);

        // Length is limited by the buffer (pbuf)
        bool sendPacket(const ip4_addr_t &destAddr, const ip4_port_t destPort, pbuf &buffer);

    };
}

#endif //RTPS_UDPDRIVER_H

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

        bool createUdpConnection(const ip4_addr_t &addr, ip4_port_t port, udp_rx_func_t callback);

        // Length is limited by the buffer (pbuf)
        bool sendPacket(const ip4_addr_t &destAddr, ip4_port_t destPort, pbuf &buffer);

    };
}

#endif //RTPS_UDPDRIVER_H

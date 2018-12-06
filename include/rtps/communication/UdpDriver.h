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
#include "rtps/common/types.h"

#include <array>

namespace rtps {

    class UdpDriver {
    private:
        std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> m_conns;
        std::size_t m_numConns = 0;

    public:
        typedef void (*udp_rx_func_t)(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, Ip4Port_t port);

        const rtps::UdpConnection* createUdpConnection(Ip4Port_t receivePort, udp_rx_func_t callback, void* args);
        bool joinMultiCastGroup(ip4_addr_t addr) const;
        bool sendPacket(const UdpConnection& conn, ip4_addr_t& destAddr, Ip4Port_t destPort, pbuf& buffer);

    };
}

#endif //RTPS_UDPDRIVER_H

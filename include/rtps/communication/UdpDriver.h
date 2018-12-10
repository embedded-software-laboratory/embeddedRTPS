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
#include "rtps/storages/PBufWrapper.h"

#include <array>

namespace rtps {

    class UdpDriver {

    public:
        struct PacketInfo{
            UdpDriver* transport;
            Ip4Port_t srcPort;
            ip4_addr_t destAddr;
            Ip4Port_t destPort;
            PBufWrapper buffer;
        };

        typedef void (*udpRxFunc_fp)(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, Ip4Port_t port);


        UdpDriver(udpRxFunc_fp callback, void* args);

        bool joinMultiCastGroup(ip4_addr_t addr) const;

        void sendFunction(PacketInfo& info);
        static void sendFunctionJumppad(void* packetInfo);
    private:
        std::array<UdpConnection, Config::MAX_NUM_UDP_CONNECTIONS> m_conns;
        std::size_t m_numConns = 0;
        udpRxFunc_fp m_rxCallback = nullptr;
        void* m_callbackArgs = nullptr;

        bool sendPacket(const UdpConnection& conn, ip4_addr_t& destAddr, Ip4Port_t destPort, pbuf& buffer);
        const rtps::UdpConnection* createUdpConnection(Ip4Port_t receivePort);
    };
}

#endif //RTPS_UDPDRIVER_H

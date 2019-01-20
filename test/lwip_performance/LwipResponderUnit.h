/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RESPONDERUNIT_H
#define RTPS_RESPONDERUNIT_H

#include "rtps/entities/Domain.h"

#include <vector>

namespace rtps {
    namespace tests {

        class LwipResponderUnit {
        public:
            explicit LwipResponderUnit();

            void run();

        private:
            udp_pcb* m_pcb;
            uint16_t m_recvPort;

            void prepareLwip();
            static void responderJumppad(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port);
            void responderCallback(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port);
        };
    }
}

#endif //RTPS_RESPONDERUNIT_H

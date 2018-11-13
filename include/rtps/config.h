/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CONFIG_H
#define RTPS_CONFIG_H

#include "rtps/types.h"

namespace rtps {

#define IS_LITTLE_ENDIAN 1

    constexpr ip4_addr transforIP4ToU32(uint8_t, uint8_t, uint8_t, uint8_t);

    namespace Config {
        const VendorId_t VENDOR_ID = {13, 37};
        const uint8_t DOMAIN_ID = 1; // 230 possible with UDP
        const std::array<uint8_t, 4> IP_ADDRESS = {192,168,0,42};

        const int SPDP_RESEND_PERIOD_MS = 500;
        const int SPDP_WRITER_STACKSIZE = 500;
        const int SPDP_WRITER_PRIO = 3;
        const Behavior::Duration_t SPDP_LEASE_DURATION = {100, 0};


        const int STATELESS_WRITER_NUM_LOCATORS = 5;
        const int NUM_UNICAST_LOCATORS = 10;
        const int NUM_MULTICAST_LOCATORS = 5;

        const int MAX_NUM_UDP_CONNECTIONS = 20;

        const int THREAD_POOL_NUM_WRITERS = 2;
        const int THREAD_POOL_WRITER_STACKSIZE = 1024;
        const int THREAD_POOL_WRITER_PRIO = 3;
        const int THREAD_POOL_WORKLOAD_QUEUE_LENGTH = 100;
    }
}

#endif //RTPS_CONFIG_H

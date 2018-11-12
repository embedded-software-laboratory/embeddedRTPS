/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CONFIG_H
#define RTPS_CONFIG_H

#include "rtps/types.h"
namespace rtps {

#define IS_LITTLE_ENDIAN 1

    namespace Config {
        const VendorId_t VENDOR_ID = {13, 37};

        const int SPDP_RESEND_PERIOD_MS = 500;
        const int SPDP_WRITER_STACKSIZE = 500;
        const int SPDP_WRITER_PRIO = 3;


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

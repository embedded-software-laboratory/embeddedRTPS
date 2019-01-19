/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CONFIG_H
#define RTPS_CONFIG_H

#include "rtps/common/types.h"

namespace rtps {

#define IS_LITTLE_ENDIAN 1


    namespace Config {
        const VendorId_t VENDOR_ID = {13, 37};
        const std::array<uint8_t, 4> IP_ADDRESS = {192,168,0,42};
        const GuidPrefix_t BASE_GUID_PREFIX{1,2,3,4,5,6,7,8,9,10,11};

        const uint8_t DOMAIN_ID = 0; // 230 possible with UDP
        const uint8_t NUM_STATELESS_WRITERS = 10;
        const uint8_t NUM_STATELESS_READERS = 10;
        const uint8_t NUM_STATEFULL_READERS = 10;
        const uint8_t NUM_STATEFULL_WRITERS = 10;
        const uint8_t MAX_NUM_PARTICIPANTS = 3;
        const uint8_t NUM_WRITERS_PER_PARTICIPANT = 10;
        const uint8_t NUM_READERS_PER_PARTICIPANT = 10;
        const uint8_t NUM_WRITER_PROXIES_PER_READER = 10;
        const uint8_t NUM_READER_PROXIES_PER_WRITER = 10;

        const uint8_t HISTORY_SIZE = 15;

        const uint8_t MAX_TYPENAME_LENGTH = 20;
        const uint8_t MAX_TOPICNAME_LENGTH = 20;

        const uint16_t SPDP_RESEND_PERIOD_MS = 10000;
        const uint16_t SPDP_WRITER_STACKSIZE = 500;
        const uint8_t SPDP_WRITER_PRIO = 3;
        const uint8_t SPDP_MAX_NUMBER_FOUND_PARTICIPANTS = 10;
        const uint8_t SPDP_MAX_NUM_LOCATORS = 10;
        const Duration_t SPDP_LEASE_DURATION = {100, 0};

        const int MAX_NUM_UDP_CONNECTIONS = 20;

        const int THREAD_POOL_NUM_WRITERS = 1;
        const int THREAD_POOL_NUM_READERS = 1;
        const int THREAD_POOL_WRITER_STACKSIZE = 1024;
        const int THREAD_POOL_READER_STACKSIZE = 1024;
        const int THREAD_POOL_WRITER_PRIO = 3;
        const int THREAD_POOL_READER_PRIO = 3;
        const int THREAD_POOL_WORKLOAD_QUEUE_LENGTH = 100;
    }
}

#endif //RTPS_CONFIG_H

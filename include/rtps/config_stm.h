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
        const std::array<uint8_t, 4> IP_ADDRESS = {192,168,0,47};
        const GuidPrefix_t BASE_GUID_PREFIX{1,2,3,4,5,6,7,8,9,10,12};

        const uint8_t DOMAIN_ID = 0; // 230 possible with UDP
        const uint8_t NUM_STATELESS_WRITERS = 2;
        const uint8_t NUM_STATELESS_READERS = 2;
        const uint8_t NUM_STATEFULL_READERS = 2;
        const uint8_t NUM_STATEFULL_WRITERS = 2;
        const uint8_t MAX_NUM_PARTICIPANTS = 1;
        const uint8_t NUM_WRITERS_PER_PARTICIPANT = 4;
        const uint8_t NUM_READERS_PER_PARTICIPANT = 4;
        const uint8_t NUM_WRITER_PROXIES_PER_READER = 3;
        const uint8_t NUM_READER_PROXIES_PER_WRITER = 3;

        const uint8_t HISTORY_SIZE = 10;

        const uint8_t MAX_TYPENAME_LENGTH = 20;
        const uint8_t MAX_TOPICNAME_LENGTH = 20;

        const int HEARTBEAT_STACKSIZE = 1200; // byte
        const int THREAD_POOL_WRITER_STACKSIZE = 1100; // byte
        const int THREAD_POOL_READER_STACKSIZE = 1600; // byte
        const uint16_t SPDP_WRITER_STACKSIZE = 550; // byte

        const uint16_t SF_WRITER_HB_PERIOD_MS = 4000;
        const uint16_t SPDP_RESEND_PERIOD_MS = 10000;
        const uint8_t SPDP_WRITER_PRIO = 3;
        const uint8_t SPDP_MAX_NUMBER_FOUND_PARTICIPANTS = 5;
        const uint8_t SPDP_MAX_NUM_LOCATORS = 5;
        const Duration_t SPDP_LEASE_DURATION = {100, 0};

        const int MAX_NUM_UDP_CONNECTIONS = 10;

        const int THREAD_POOL_NUM_WRITERS = 1;
        const int THREAD_POOL_NUM_READERS = 1;
        const int THREAD_POOL_WRITER_PRIO = 3;
        const int THREAD_POOL_READER_PRIO = 3;
        const int THREAD_POOL_WORKLOAD_QUEUE_LENGTH = 10;

        constexpr int OVERALL_HEAP_SIZE = 	THREAD_POOL_NUM_WRITERS * THREAD_POOL_WRITER_STACKSIZE +
											THREAD_POOL_NUM_READERS * THREAD_POOL_READER_STACKSIZE +
											MAX_NUM_PARTICIPANTS * SPDP_WRITER_STACKSIZE +
											NUM_STATEFULL_WRITERS * HEARTBEAT_STACKSIZE;
    }
}

#endif //RTPS_CONFIG_H

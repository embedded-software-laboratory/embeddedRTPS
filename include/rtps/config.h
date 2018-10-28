/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_CONFIG_H
#define RTPS_CONFIG_H

namespace Config{
    static const int MAX_NUM_UDP_CONNECTIONS = 20;

    static const int THREAD_POOL_NUM_WRITERS = 2;
    static const int THREAD_POOL_WRITER_STACKSIZE = 1024;
    static const int THREAD_POOL_WRITER_PRIO = 3;
    static const int THREAD_POOL_WORKLOAD_QUEUE_LENGTH = 100;
}

#endif //RTPS_CONFIG_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "rtps/types.h"
#include "rtps/ThreadPool.h"
#include "rtps/entities/StatelessWriter.h"

namespace rtps{
    class SPDPAgent{
    public:
        explicit SPDPAgent(ThreadPool& pool);
        void start();
        void stop();

    private:
        StatelessWriter writer;
        bool running = false;
        std::array<uint8_t, 4> testData{'s', 'p', 'd', 'p'};

        static void run(void* args);
    };
}

#endif //RTPS_SPDP_H

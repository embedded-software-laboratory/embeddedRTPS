/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "rtps/types.h"
#include "rtps/ThreadPool.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/StatelessWriter.h"

#include "ucdr/microcdr.h"

namespace rtps{
    class SPDPAgent{
    public:
        SPDPAgent(ThreadPool& pool, Participant& participant);
        void start();
        void stop();

    private:
        Participant& participant;
        StatelessWriter writer;
        bool running = false;
        std::array<uint8_t, 400> buffer{};
        ucdrBuffer microbuffer;

        void init();
        void addInlineQos();
        void addParticipantParameters();
        void endCurrentList();


        static void run(void* args);

    };
}

#endif //RTPS_SPDP_H

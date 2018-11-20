/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "rtps/types.h"
#include "rtps/ThreadPool.h"
#include "rtps/entities/StatelessWriter.h"

#include "ucdr/microcdr.h"

namespace rtps{
    class Participant;
    class SPDPAgent{
    public:
        void init(Participant& participant);
        void start();
        void stop();

    private:
        Participant* mp_participant = nullptr;
        Writer* mp_writer = nullptr;
        bool m_running = false;
        std::array<uint8_t, 400> m_buffer{};
        ucdrBuffer m_microbuffer;

        void addInlineQos();
        void addParticipantParameters();
        void endCurrentList();


        static void run(void* args);

    };
}

#endif //RTPS_SPDP_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/ParticipantProxyData.h"
#include "lwip/sys.h"
#include "ucdr/microcdr.h"

namespace rtps{
    class Participant;
    class BuiltInEndpoints;
    class Writer;
    class Reader;
    class ReaderCacheChange;

    class SPDPAgent{
    public:
        ~SPDPAgent();
        void init(Participant& participant, BuiltInEndpoints& endpoints);
        void start();
        void stop();

    private:
        Participant* mp_participant = nullptr;
        std::array<ParticipantProxyData, Config::SPDP_MAX_NUMBER_FOUND_PARTICIPANTS> m_foundParticipants;
        Writer* mp_writer = nullptr;
        Reader* mp_reader = nullptr;
        bool m_running = false;
        std::array<uint8_t, 400> m_outputBuffer{}; // TODO check required size
        std::array<uint8_t, 400> m_inputBuffer{};
        ParticipantProxyData m_proxyDataBuffer{};
        ucdrBuffer m_microbuffer{};

        sys_mutex_t m_mutex;
        bool initialized = false;
        static void receiveCallback(void* callee, ReaderCacheChange& cacheChange);
        void handleSPDPPackage(ReaderCacheChange& cacheChange);

        void addInlineQos();
        void addParticipantParameters();
        void endCurrentList();

        static void runBroadcast(void *args);


    };
}

#endif //RTPS_SPDP_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/ParticipantProxyData.h"
#include "rtps/discovery/BuiltInEndpoints.h"
#include "lwip/sys.h"
#include "ucdr/microcdr.h"

namespace rtps{
    class Participant;
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
        BuiltInEndpoints m_buildInEndpoints;
        bool m_running = false;
        std::array<uint8_t, 400> m_outputBuffer{}; // TODO check required size
        std::array<uint8_t, 400> m_inputBuffer{};
        ParticipantProxyData m_proxyDataBuffer{};
        ucdrBuffer m_microbuffer{};

        sys_mutex_t m_mutex;
        bool initialized = false;
        static void receiveCallback(void* callee, const ReaderCacheChange& cacheChange);
        void handleSPDPPackage(const ReaderCacheChange& cacheChange);
        void configureEndianessAndOptions(ucdrBuffer& buffer);
        void processProxyData();
        void addProxiesForBuiltInEndpoints();

        void addInlineQos();
        void addParticipantParameters();
        void endCurrentList();

        static void runBroadcast(void *args);


    };
}

#endif //RTPS_SPDP_H

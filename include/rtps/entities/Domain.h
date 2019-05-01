/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DOMAIN_H
#define RTPS_DOMAIN_H

#include "rtps/config.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/StatefulReader.h"
#include "rtps/entities/StatefulWriter.h"
#include "rtps/entities/StatelessReader.h"
#include "rtps/entities/StatelessWriter.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"

namespace rtps{
    class Domain{
    public:

        Domain();

        bool completeInit();
        void stop();

        Participant* createParticipant();
        Writer* createWriter(Participant& part, const char* topicName, const char* typeName, bool reliable);
        Reader* createReader(Participant& part, const char* topicName, const char* typeName, bool reliable);

    private:
        ThreadPool m_threadPool;
        UdpDriver m_transport;
        std::array<Participant, Config::MAX_NUM_PARTICIPANTS> m_participants;
        const uint8_t PARTICIPANT_START_ID = 0;
        ParticipantId_t m_nextParticipantId = PARTICIPANT_START_ID;

        std::array<StatelessWriter, Config::NUM_STATELESS_WRITERS> m_statelessWriters;
        uint8_t m_numStatelessWriters = 0;
        std::array<StatelessReader, Config::NUM_STATELESS_READERS> m_statelessReaders;
        uint8_t m_numStatelessReaders = 0;
        std::array<StatefulReader, Config::NUM_STATEFUL_READERS> m_statefulReaders;
        uint8_t m_numStatefulReaders = 0;
        std::array<StatefulWriter, Config::NUM_STATEFUL_WRITERS> m_statefulWriters;
        uint8_t m_numStatefulWriters = 0;

        bool m_initComplete = false;

        void receiveCallback(const PacketInfo& packet);
        GuidPrefix_t generateGuidPrefix(ParticipantId_t id) const;
        void createBuiltinWritersAndReaders(Participant &part);
        void registerPort(const Participant& part);
        static void receiveJumppad(void* callee, const PacketInfo& packet);
    };
}

#endif //RTPS_DOMAIN_H

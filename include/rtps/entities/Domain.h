/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DOMAIN_H
#define RTPS_DOMAIN_H

#include "rtps/config.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/StatefullReader.h"
#include "rtps/entities/StatefullWriter.h"
#include "rtps/entities/StatelessReader.h"
#include "rtps/entities/StatelessWriter.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"

namespace rtps{
    class Domain{
    public:

        Domain();

        bool start();
        void stop();

        void receiveCallback(const PacketInfo& packet);

        Participant* createParticipant();
        Writer* createWriter(Participant& part, char* topicName, char* typeName, bool reliable);
        Reader* createReader(Participant& part, char* topicName, char* typeName, bool reliable);

    private:
        ThreadPool m_threadPool;
        UdpDriver m_transport;
        std::array<Participant, Config::MAX_NUM_PARTICIPANTS> m_participants;
        const uint8_t PARTICIPANT_START_ID = 1;
        ParticipantId_t m_nextParticipantId = PARTICIPANT_START_ID;

        std::array<StatelessWriter, Config::NUM_STATELESS_WRITERS> m_statelessWriters;
        uint8_t m_numStatelessWriters = 0;
        std::array<StatelessReader, Config::NUM_STATELESS_READERS> m_statelessReaders;
        uint8_t m_numStatelessReaders = 0;
        std::array<StatefullReader, Config::NUM_STATEFULL_READERS> m_statefullReaders;
        uint8_t m_numStatefullReaders = 0;
        std::array<StatefullWriter, Config::NUM_STATEFULL_WRITERS> m_statefullWriters;
        uint8_t m_numStatefullWriters = 0;

        GuidPrefix_t generateGuidPrefix(ParticipantId_t id) const;
        void addDefaultWriterAndReader(Participant& part);
        void registerPort(Participant& part);
    };
}

#endif //RTPS_DOMAIN_H

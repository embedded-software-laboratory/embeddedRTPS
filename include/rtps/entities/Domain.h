/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DOMAIN_H
#define RTPS_DOMAIN_H

#include "rtps/config.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/StatelessWriter.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"

namespace rtps{
    class Domain{
    public:

        Domain();

        bool start();
        void stop();

        void receiveCallback(PBufWrapper buffer);

        Participant* createParticipant();
        Writer* createWriter(Participant& part, Locator_t locator, bool reliable);

    private:
        ThreadPool pool;
        std::array<Participant, Config::MAX_NUM_PARTICIPANTS> m_participants;
        participantId_t m_nextParticipantId = 1;

        std::array<StatelessWriter, Config::NUM_STATELESS_WRITERS> m_statelessWriters;
        uint8_t m_numStatelessWriters = 0;

        GuidPrefix_t generateGuidPrefix(participantId_t id) const;
        void addDefaultWriterAndReader(Participant& part);
    };
}

#endif //RTPS_DOMAIN_H

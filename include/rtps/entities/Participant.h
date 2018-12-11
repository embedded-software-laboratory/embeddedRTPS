/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PARTICIPANT_H
#define RTPS_PARTICIPANT_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/SPDPAgent.h"
#include "rtps/messages/MessageReceiver.h"

namespace rtps{

    class Writer;
    class Reader;

    class Participant{
    public:
         GuidPrefix_t guidPrefix;
         ParticipantId_t participantId;

        Participant();
        explicit Participant(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId);
        ~Participant();
        bool isValid();

        std::array<uint8_t, 3> getNextUserEntityKey();
        Writer* addUserWriter(Writer* writer);
        Reader* addUserReader(Reader* reader);
        void addBuiltInEndpoints(BuiltInEndpoints& endpoints);
        void newMessage(const uint8_t* data, DataSize_t size);

    private:
        MessageReceiver receiver;
        std::array<uint8_t, 3> m_nextUserEntityId{0,0,0};
        SPDPAgent m_spdpAgent;
    };
}

#endif //RTPS_PARTICIPANT_H

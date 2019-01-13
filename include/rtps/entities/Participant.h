/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PARTICIPANT_H
#define RTPS_PARTICIPANT_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/SPDPAgent.h"
#include "rtps/discovery/SEDPAgent.h"
#include "rtps/messages/MessageReceiver.h"

namespace rtps{

    class Writer;
    class Reader;

    class Participant{
    public:
         GuidPrefix_t m_guidPrefix;
         ParticipantId_t m_participantId;

        Participant();
        explicit Participant(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId);

        // Not allowed because the message receiver contains a pointer to the participant
        Participant(const Participant&) = delete;
        Participant(Participant&&) = delete;
        Participant& operator=(const Participant&) = delete;
        Participant& operator=(Participant&&) = delete;

        ~Participant();
        bool isValid();

        void reuse(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId);

        std::array<uint8_t, 3> getNextUserEntityKey();

        //! Not-thread-safe function to add a writer
        Writer* addWriter(Writer* writer);
        //! Not-thread-safe function to add a reader
        Reader* addReader(Reader* reader);

        //! (Probably) Thread safe if writers cannot be removed
        Writer* getWriter(EntityId_t id) const;
        Writer* getWriter(const char* topic, const char* type) const;
        //! (Probably) Thread safe if readers cannot be removed
        Reader* getReader(EntityId_t id) const;
        Reader* getReader(const char* topic, const char* type) const;

        bool addNewRemoteParticipant(ParticipantProxyData& remotePart);
        const ParticipantProxyData* findRemoteParticipant(const GuidPrefix_t& prefix) const;

        MessageReceiver* getMessageReceiver();

        void addBuiltInEndpoints(BuiltInEndpoints& endpoints);
        void newMessage(const uint8_t* data, DataSize_t size);

    private:
        MessageReceiver m_receiver;
        bool m_hasBuilInEndpoints = false;
        std::array<uint8_t, 3> m_nextUserEntityId{0,0,0};
        std::array<Writer*, Config::NUM_WRITERS_PER_PARTICIPANT> m_writers{nullptr};
        uint8_t m_numWriters = 0;
        std::array<Reader*, Config::NUM_READERS_PER_PARTICIPANT> m_readers{nullptr};
        uint8_t m_numReaders = 0;
        const bool m_pushMode = false; // No other mode supported atm

        std::array<ParticipantProxyData, Config::SPDP_MAX_NUMBER_FOUND_PARTICIPANTS> m_foundParticipants;

        SPDPAgent m_spdpAgent;
        SEDPAgent m_sedpAgent;
    };
}

#endif //RTPS_PARTICIPANT_H

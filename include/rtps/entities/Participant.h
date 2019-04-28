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

        // Actually the only two function that should be used by the user
        bool registerOnNewPublisherMatchedCallback(void (*callback)(void* arg), void* args);
        bool registerOnNewSubscriberMatchedCallback(void (*callback)(void* arg), void* args);

        //! Not-thread-safe function to add a writer
        Writer* addWriter(Writer* writer);
        //! Not-thread-safe function to add a reader
        Reader* addReader(Reader* reader);

        //! (Probably) Thread safe if writers cannot be removed
        Writer* getWriter(EntityId_t id) const;
        Writer* getMatchingWriter(const TopicData& topicData) const;
        //! (Probably) Thread safe if readers cannot be removed
        Reader* getReader(EntityId_t id) const;
        Reader* getMatchingReader(const TopicData& topicData) const;

        bool addNewRemoteParticipant(const ParticipantProxyData& remotePart);
        bool removeRemoteParticipant(const GuidPrefix_t& prefix);
        const ParticipantProxyData* findRemoteParticipant(const GuidPrefix_t& prefix);

        MessageReceiver* getMessageReceiver();

        void addBuiltInEndpoints(BuiltInEndpoints& endpoints);
        void newMessage(const uint8_t* data, DataSize_t size);

    private:
        MessageReceiver m_receiver;
        bool m_hasBuilInEndpoints = false;
        std::array<uint8_t, 3> m_nextUserEntityId{{0,0,1}};
        std::array<Writer*, Config::NUM_WRITERS_PER_PARTICIPANT> m_writers{};
        uint8_t m_numWriters = 0;
        std::array<Reader*, Config::NUM_READERS_PER_PARTICIPANT> m_readers{};
        uint8_t m_numReaders = 0;

        MemoryPool<ParticipantProxyData, Config::SPDP_MAX_NUMBER_FOUND_PARTICIPANTS> m_remoteParticipants;

        SPDPAgent m_spdpAgent;
        SEDPAgent m_sedpAgent;
    };
}

#endif //RTPS_PARTICIPANT_H

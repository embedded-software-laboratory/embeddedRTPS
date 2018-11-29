/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PARTICIPANT_H
#define RTPS_PARTICIPANT_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/discovery/SPDP.h"
#include "rtps/messages/MessageReceiver.h"

namespace rtps{

    class Writer;
    class Reader;

    class Participant{
    public:
         GuidPrefix_t guidPrefix;
         participantId_t participantId;

        Participant();
        explicit Participant(const GuidPrefix_t& guidPrefix, participantId_t participantId);
        ~Participant();
        bool isValid();

        std::array<uint8_t, 3> getNextUserEntityKey();
        Writer* addUserWriter(Writer& writer);
        void addSPDPWriter(Writer& writer);
        void addSPDPReader(Reader& reader);
        Writer* getSPDPWriter();
        Reader* getSPDPReader();
        void newMessage(const uint8_t* data, uint16_t len);

    private:
        std::array<Writer*, Config::NUM_WRITERS_PER_PARTICIPANT> m_writers{nullptr};
        uint8_t m_numWriters = 0;
        std::array<Writer*, Config::NUM_READERS_PER_PARTICIPANT> m_readers{nullptr};
        uint8_t m_numReaders = 0;
        MessageReceiver receiver;

        std::array<uint8_t, 3> m_nextUserEntityId{0,0,0};
        Writer* mp_SPDPWriter = nullptr;
        Reader* mp_SPDPReader = nullptr;
        SPDPAgent m_spdpAgent;
    };
}

#endif //RTPS_PARTICIPANT_H

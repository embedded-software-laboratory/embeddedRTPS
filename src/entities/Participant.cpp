/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Participant.h"

#include "rtps/entities/Writer.h"
#include "rtps/entities/Reader.h"
#include "rtps/messages/MessageReceiver.h"

using rtps::Participant;

Participant::Participant() : guidPrefix(GUIDPREFIX_UNKNOWN), participantId(PARTICIPANT_ID_INVALID),
                             receiver(GUIDPREFIX_UNKNOWN){

}
Participant::Participant(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId)
        : guidPrefix(guidPrefix), participantId(participantId),
          receiver(guidPrefix){

}

Participant::~Participant() {
    m_spdpAgent.stop();
}

bool Participant::isValid(){
    return participantId != PARTICIPANT_ID_INVALID;
}

std::array<uint8_t, 3> Participant::getNextUserEntityKey(){
    const auto result = m_nextUserEntityId;

    ++m_nextUserEntityId[2];
    if(m_nextUserEntityId[2] == 0){
        ++m_nextUserEntityId[1];
        if(m_nextUserEntityId[1] == 0){
            ++m_nextUserEntityId[0];
        }
    }
    return result;
}

rtps::Writer* Participant::addUserWriter(Writer& writer){
    if(receiver.addWriter(writer)){
        return &writer;
    }else{
        return nullptr;
    }
}

void Participant::addBuiltInEndpoints(BuiltInEndpoints& endpoints){
    receiver.addBuiltInEndpoints(endpoints);

    m_spdpAgent.init(*this, endpoints);
    m_spdpAgent.start();
}

void Participant::newMessage(const uint8_t* data, DataSize_t size){
    receiver.processMessage(data, size);
}
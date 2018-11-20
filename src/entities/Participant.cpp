/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Participant.h"

using rtps::Participant;

Participant::Participant() : guidPrefix(GUIDPREFIX_UNKNOWN), participantId(PARTICIPANT_ID_INVALID){};
Participant::Participant(const GuidPrefix_t& guidPrefix, participantId_t participantId)
        : guidPrefix(guidPrefix), participantId(participantId){};

Participant::~Participant() {
    spdpAgent.stop();
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
    if(numWriters >= m_writers.size()){
        return nullptr;
    }
    m_writers[numWriters] = &writer;
    ++numWriters;
    return &writer;
}

void Participant::addSPDPWriter(rtps::Writer &writer) {
    m_SPDPWriter = &writer;
    spdpAgent.init(*this);
    spdpAgent.start();
}

rtps::Writer* Participant::getSPDPWriter() {
    return m_SPDPWriter;
}
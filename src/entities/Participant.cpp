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
    if(m_numWriters >= m_writers.size()){
        return nullptr;
    }
    m_writers[m_numWriters] = &writer;
    ++m_numWriters;
    return &writer;
}

void Participant::addSPDPWriter(rtps::Writer &writer) {
    mp_SPDPWriter = &writer;
    m_spdpAgent.init(*this);
    m_spdpAgent.start();
}

rtps::Writer* Participant::getSPDPWriter() {
    return mp_SPDPWriter;
}
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
Participant::Participant(const GuidPrefix_t& guidPrefix, participantId_t participantId)
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
    if(m_numWriters >= m_writers.size()){
        return nullptr;
    }
    m_writers[m_numWriters] = &writer;
    ++m_numWriters;
    return &writer;
}

void Participant::addSPDPWriter(Writer &writer) {
    mp_SPDPWriter = &writer;
    if(mp_SPDPReader != nullptr){
        m_spdpAgent.init(*this);
        m_spdpAgent.start();
    }
}

rtps::Writer* Participant::getSPDPWriter() {
    return mp_SPDPWriter;
}

void Participant::addSPDPReader(Reader &reader) {
    mp_SPDPReader = &reader;
    if(mp_SPDPWriter != nullptr){
        m_spdpAgent.init(*this);
        m_spdpAgent.start();
    }
    receiver.addReader(reader);
}

rtps::Reader* Participant::getSPDPReader() {
    return mp_SPDPReader;
}

void Participant::newMessage(const uint8_t* data, uint16_t size){
    receiver.processMessage(data, size);
}
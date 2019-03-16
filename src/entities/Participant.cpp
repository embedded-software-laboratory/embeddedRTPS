/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Participant.h"

#include "rtps/entities/Writer.h"
#include "rtps/entities/Reader.h"
#include "rtps/messages/MessageReceiver.h"

using rtps::Participant;

Participant::Participant() : m_guidPrefix(GUIDPREFIX_UNKNOWN), m_participantId(PARTICIPANT_ID_INVALID),
                             m_receiver(this){

}
Participant::Participant(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId)
        : m_guidPrefix(guidPrefix), m_participantId(participantId),
          m_receiver(this){

}

Participant::~Participant() {
    m_spdpAgent.stop();
}

void Participant::reuse(const GuidPrefix_t& guidPrefix, ParticipantId_t participantId){
    m_guidPrefix = guidPrefix;
    m_participantId = participantId;
}

bool Participant::isValid(){
    return m_participantId != PARTICIPANT_ID_INVALID;
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

rtps::Writer* Participant::addWriter(Writer* pWriter){
    if(pWriter != nullptr && m_numWriters != m_writers.size()){
        m_writers[m_numWriters++] = pWriter;
        if(m_hasBuilInEndpoints){
            m_sedpAgent.addWriter(*pWriter);
        }
        return pWriter;
    }else{
        return nullptr;
    }
}

rtps::Reader* Participant::addReader(Reader* pReader){
    if(pReader != nullptr && m_numReaders != m_readers.size()){
        m_readers[m_numReaders++] = pReader;
        if(m_hasBuilInEndpoints){
            m_sedpAgent.addReader(*pReader);
        }
        return pReader;
    }else{
        return nullptr;
    }
}


rtps::Writer* Participant::getWriter(EntityId_t id) const{
    for(uint8_t i=0; i < m_numWriters; ++i){
        if(m_writers[i]->m_attributes.endpointGuid.entityId == id){
            return m_writers[i];
        }
    }
    return nullptr;
}

rtps::Reader* Participant::getReader(EntityId_t id) const{
    for(uint8_t i=0; i < m_numReaders; ++i){
        if(m_readers[i]->m_attributes.endpointGuid.entityId == id){
            return m_readers[i];
        }
    }
    return nullptr;
}

rtps::Writer* Participant::getMatchingWriter(const TopicData& readerTopicData) const{
    for(uint8_t i=0; i < m_numWriters; ++i){
        if(m_writers[i]->m_attributes.matchesTopicOf(readerTopicData) &&
                (readerTopicData.reliabilityKind == ReliabilityKind_t::BEST_EFFORT ||
                 m_writers[i]->m_attributes.reliabilityKind == ReliabilityKind_t::RELIABLE)){
            return m_writers[i];
        }
    }
    return nullptr;
}

rtps::Reader* Participant::getMatchingReader(const TopicData& writerTopicData) const{
    for(uint8_t i=0; i < m_numReaders; ++i){
        if(m_readers[i]->m_attributes.matchesTopicOf(writerTopicData) &&
                (writerTopicData.reliabilityKind == ReliabilityKind_t::RELIABLE ||
                 m_readers[i]->m_attributes.reliabilityKind == ReliabilityKind_t::BEST_EFFORT)){
            return m_readers[i];
        }
    }
    return nullptr;
}

bool Participant::addNewRemoteParticipant(ParticipantProxyData& remotePart){
    for(auto& partProxy : m_foundParticipants) {
        if (partProxy.m_guid.prefix.id == GUIDPREFIX_UNKNOWN.id) {
            partProxy = remotePart;
            return true;
        }
    }
    return false;
}

const rtps::ParticipantProxyData* Participant::findRemoteParticipant(const GuidPrefix_t& prefix) const{
    for(auto& partProxy : m_foundParticipants){
        if(partProxy.m_guid.prefix == prefix){
            return &partProxy;
        }
    }
    return nullptr;
}

rtps::MessageReceiver* Participant::getMessageReceiver(){
    return &m_receiver;
}

void Participant::addBuiltInEndpoints(BuiltInEndpoints& endpoints){
    m_hasBuilInEndpoints = true;
    m_spdpAgent.init(*this, endpoints);
    m_spdpAgent.start();
    m_sedpAgent.init(*this, endpoints);

    // This needs to be done after initializing the agents
    addWriter(endpoints.spdpWriter);
    addReader(endpoints.spdpReader);
    addWriter(endpoints.sedpPubWriter);
    addReader(endpoints.sedpPubReader);
    addWriter(endpoints.sedpSubWriter);
    addReader(endpoints.sedpSubReader);

    // TODO add all existing writers and readers in case there were added before the endpoints
}

void Participant::newMessage(const uint8_t* data, DataSize_t size){
    m_receiver.processMessage(data, size);
}
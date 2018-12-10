/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Domain.h"
#include "rtps/utils/udpUtils.h"

using rtps::Domain;

Domain::Domain() : m_threadPool(*this), m_transport(ThreadPool::readCallback, &m_threadPool){
    //TODO move avoid from here
    LOCK_TCPIP_CORE();
    m_transport.joinMultiCastGroup(transformIP4ToU32(239, 255, 0, 1));
    UNLOCK_TCPIP_CORE();
}

bool Domain::start(){
    bool started = m_threadPool.startThreads();
    if(!started){
        printf("Failed starting threads");
    }
    return started;
}

void Domain::stop(){
    m_threadPool.stopThreads();
}

void Domain::receiveCallback(PBufWrapper buffer, Ip4Port_t destPort){
    if(buffer.firstElement->next != nullptr){
        printf("Domain: Cannot handle multiple elements chained. You might want to increase PBUF_POOL_BUFSIZE\n");
    }
    if(isMultiCastPort(destPort)){
        // Pass to all
        for(auto i=0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
            m_participants[i].newMessage(static_cast<uint8_t*>(buffer.firstElement->payload), buffer.firstElement->len);
        }
    }else{
        // Pass to addressed one only
        ParticipantId_t id = getParticipantIdFromUnicastPort(destPort, isUserPort(destPort));
        if(id != PARTICIPANT_ID_INVALID){
            m_participants[id-PARTICIPANT_START_ID].newMessage(static_cast<uint8_t*>(buffer.firstElement->payload),
                                                               buffer.firstElement->len);
        }
    }
}

rtps::Participant* Domain::createParticipant(){
    for(auto& entry : m_participants){
        if(entry.participantId == PARTICIPANT_ID_INVALID){
            entry = Participant{generateGuidPrefix(m_nextParticipantId), m_nextParticipantId};
            addDefaultWriterAndReader(entry);
            registerPort(entry);
            ++m_nextParticipantId;
            return &entry;
        }
    }
    return nullptr;
}

void Domain::addDefaultWriterAndReader(Participant& part) {
    //SPDP
    StatelessWriter& spdpWriter = m_statelessWriters[m_numStatelessWriters++];
    spdpWriter.init(TopicKind_t::WITH_KEY, &m_threadPool, part.guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER,
                    m_transport, getBuiltInMulticastPort());
    spdpWriter.addNewMatchedReader(ReaderLocator(ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER, getDefaultSendMulticastLocator()));

    StatelessReader& spdpReader = m_statelessReaders[m_numStatelessReaders++];
    spdpReader.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;


    BuiltInEndpoints endpoints{};
    endpoints.spdpWriter = &spdpWriter;
    endpoints.spdpReader = &spdpReader;

    part.addBuiltInEndpoints(endpoints);
}

void Domain::registerPort(Participant& /*part*/){
    // TODO Issue #10
}

rtps::Writer* Domain::createWriter(Participant& part, bool reliable){
    if(reliable){
        // TODO StatefulWriter
        return nullptr;
    }else{
        // TODO Distinguish WithKey and NoKey (Also changes EntityKind)
        m_statelessWriters[m_numStatelessWriters].init(TopicKind_t::NO_KEY, &m_threadPool, part.guidPrefix,
                                                       {part.getNextUserEntityKey(), EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY},
                                                       m_transport, getUserUnicastPort(part.participantId));

        return &m_statelessWriters[m_numStatelessWriters++];
    }
}

rtps::GuidPrefix_t Domain::generateGuidPrefix(ParticipantId_t id) const{
    // TODO
    GuidPrefix_t prefix{1,2,3,4,5,6,7,8,9,10,11, *reinterpret_cast<uint8_t*>(&id)};
    return prefix;
}

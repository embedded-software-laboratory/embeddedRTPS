/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Domain.h"
#include "rtps/utils/udpUtils.h"

using rtps::Domain;

Domain::Domain() : m_threadPool(*this){

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

void Domain::receiveCallback(PBufWrapper buffer){
    printf("received callback");
}

rtps::Participant* Domain::createParticipant(){
    for(auto& entry : m_participants){
        if(entry.participantId == PARTICIPANT_ID_INVALID){
            entry = Participant{generateGuidPrefix(m_nextParticipantId), m_nextParticipantId};
            addDefaultWriterAndReader(entry);
            return &entry;
        }
    }
    return nullptr;
}

void Domain::addDefaultWriterAndReader(rtps::Participant &part) {
    m_statelessWriters[m_numStatelessWriters].init(TopicKind_t::WITH_KEY, getDefaultSendMulticastLocator(),
                                                   &m_threadPool, part.guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
    part.addSPDPWriter(m_statelessWriters[m_numStatelessWriters++]);
}

rtps::Writer* Domain::createWriter(Participant& part, Locator_t locator, bool reliable){
    if(reliable){
        // TODO StatefulWriter
        return nullptr;
    }else{
        // TODO Differentiate WithKey and NoKey (Also changes EntityKind)
        m_statelessWriters[m_numStatelessWriters].init(TopicKind_t::NO_KEY, locator, &m_threadPool, part.guidPrefix,
                                                       {part.getNextUserEntityKey(), EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY});

        return &m_statelessWriters[m_numStatelessWriters++];
    }
}

rtps::GuidPrefix_t Domain::generateGuidPrefix(participantId_t id) const{
    // TODO
    GuidPrefix_t prefix{1,2,3,4,5,6,7,8,9,10,11, *reinterpret_cast<uint8_t*>(&id)};
    return prefix;
}

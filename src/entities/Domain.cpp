/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Domain.h"
#include "rtps/utils/udpUtils.h"

#define DOMAIN_VERBOSE 0

using rtps::Domain;

Domain::Domain() : m_threadPool(*this), m_transport(ThreadPool::readCallback, &m_threadPool){
    //TODO move avoid from here
    m_transport.joinMultiCastGroup(transformIP4ToU32(239, 255, 0, 1));
}

bool Domain::start(){
    bool started = m_threadPool.startThreads();
    if(!started){
#if DOMAIN_VERBOSE
        printf("Failed starting threads");
#endif
    }
    return started;
}

void Domain::stop(){
    m_threadPool.stopThreads();
}

void Domain::receiveCallback(const PacketInfo& packet){
    if(packet.buffer.firstElement->next != nullptr){
#if DOMAIN_VERBOSE
        printf("Domain: Cannot handle multiple elements chained. You might want to increase PBUF_POOL_BUFSIZE\n");
#endif
    }

    if(isMultiCastPort(packet.destPort)){
        // Pass to all
#if DOMAIN_VERBOSE
        printf("Multicast to port %u\n", packet.destPort);
#endif
        for(auto i=0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
            m_participants[i].newMessage(static_cast<uint8_t*>(packet.buffer.firstElement->payload), packet.buffer.firstElement->len);
        }
    }else{
        // Pass to addressed one only
        ParticipantId_t id = getParticipantIdFromUnicastPort(packet.destPort, isUserPort(packet.destPort));
        if(id != PARTICIPANT_ID_INVALID){
#if DOMAIN_VERBOSE
            printf("Got unicast message\n");
#endif
            m_participants[id-PARTICIPANT_START_ID].newMessage(static_cast<uint8_t*>(packet.buffer.firstElement->payload),
                                                               packet.buffer.firstElement->len);
        }else{
#if DOMAIN_VERBOSE
            printf("Got message to port %u: no matching participant\n", packet.destPort);
#endif
        }
    }
}

rtps::Participant* Domain::createParticipant(){
    for(auto& entry : m_participants){
        if(entry.m_participantId == PARTICIPANT_ID_INVALID){
            entry.reuse(generateGuidPrefix(m_nextParticipantId), m_nextParticipantId);
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
    StatelessReader& spdpReader = m_statelessReaders[m_numStatelessReaders++];

    BuiltInTopicData spdpWriterAttributes;
    spdpWriterAttributes.topicName[0] = '\0';
    spdpWriterAttributes.typeName[0] = '\0';
    spdpWriterAttributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;
    spdpWriterAttributes.endpointGuid.prefix = part.m_guidPrefix;
    spdpWriterAttributes.endpointGuid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
    spdpWriterAttributes.unicastLocator = getBuiltInMulticastLocator();

    spdpWriter.init(spdpWriterAttributes, TopicKind_t::WITH_KEY, &m_threadPool, m_transport);
    spdpWriter.addNewMatchedReader(ReaderProxy{{part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER}, getBuiltInMulticastLocator()});
    spdpReader.m_guid = {part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER};

    // SEDP
    StatefullReader& sedpPubReader = m_statefullReaders[m_numStatefullReaders++];
    StatefullReader& sedpSubReader = m_statefullReaders[m_numStatefullReaders++];
    StatefullWriter& sedpPubWriter = m_statefullWriters[m_numStatefullWriters++];
    StatefullWriter& sedpSubWriter = m_statefullWriters[m_numStatefullWriters++];

    // READER
    sedpPubReader.init({part.m_guidPrefix, ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER}, m_transport, getBuiltInUnicastPort(part.m_participantId));
    sedpSubReader.init({part.m_guidPrefix, ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER}, m_transport, getBuiltInUnicastPort(part.m_participantId));

    // WRITER
    BuiltInTopicData sedpWriterAttributes;
    sedpWriterAttributes.topicName[0] = '\0';
    sedpWriterAttributes.typeName[0] = '\0';
    sedpWriterAttributes.reliabilityKind = ReliabilityKind_t::RELIABLE;
    sedpWriterAttributes.endpointGuid.prefix = part.m_guidPrefix;
    sedpWriterAttributes.unicastLocator = getBuiltInUnicastLocator(part.m_participantId);

    sedpWriterAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
    sedpPubWriter.init(sedpWriterAttributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

    sedpWriterAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
    sedpSubWriter.init(sedpWriterAttributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

    // COLLECT
    BuiltInEndpoints endpoints{};
    endpoints.spdpWriter = &spdpWriter;
    endpoints.spdpReader = &spdpReader;
    endpoints.sedpPubReader = &sedpPubReader;
    endpoints.sedpSubReader = &sedpSubReader;
    endpoints.sedpPubWriter = &sedpPubWriter;
    endpoints.sedpSubWriter = &sedpSubWriter;

    part.addBuiltInEndpoints(endpoints);
}

void Domain::registerPort(Participant& part){
    // TODO Issue #10

}

rtps::Writer* Domain::createWriter(Participant& part, char* topicName, char* typeName, bool reliable){
    // TODO Distinguish WithKey and NoKey (Also changes EntityKind)
    BuiltInTopicData attributes;

    if(strlen(topicName) > Config::MAX_TOPICNAME_LENGTH || strlen(typeName) > Config::MAX_TYPENAME_LENGTH){
        return nullptr;
    }
    strcpy(attributes.topicName, topicName);
    strcpy(attributes.typeName, typeName);
    attributes.endpointGuid.prefix = part.m_guidPrefix;
    attributes.endpointGuid.entityId = {part.getNextUserEntityKey(), EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY};
    attributes.unicastLocator = getUserUnicastLocator(part.m_participantId);

    if(reliable){
        if(m_numStatefullWriters == m_statefullWriters.size()){
            return nullptr;
        }

        attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

        StatefullWriter& writer = m_statefullWriters[m_numStatefullWriters];
        writer.init(attributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

        part.addWriter(&writer);
        return &writer;
    }else{
        if(m_numStatelessWriters == m_statelessWriters.size()){
            return nullptr;
        }

        attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

        StatelessWriter& writer = m_statelessWriters[m_numStatelessWriters];
        writer.init(attributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

        part.addWriter(&writer);
        return &writer;
    }
}


rtps::GuidPrefix_t Domain::generateGuidPrefix(ParticipantId_t id) const{
    // TODO
    GuidPrefix_t prefix{1,2,3,4,5,6,7,8,9,10,11, *reinterpret_cast<uint8_t*>(&id)};
    return prefix;
}

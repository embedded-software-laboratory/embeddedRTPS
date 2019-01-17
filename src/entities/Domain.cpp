/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/Domain.h"
#include "rtps/utils/udpUtils.h"

#define DOMAIN_VERBOSE 0

using rtps::Domain;

Domain::Domain() : m_threadPool(*this), m_transport(ThreadPool::readCallback, &m_threadPool){
    //TODO move away from here
    m_transport.createUdpConnection(getUserMulticastPort());
    m_transport.createUdpConnection(getBuiltInMulticastPort());
    m_transport.joinMultiCastGroup(transformIP4ToU32(239, 255, 0, 1));
}

bool Domain::start(){
    bool started = m_threadPool.startThreads();
    if(!started){
#if DOMAIN_VERBOSE
        printf("Domain: Failed starting threads\n");
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
        printf("Domain: Multicast to port %u\n", packet.destPort);
#endif
        for(auto i=0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
            m_participants[i].newMessage(static_cast<uint8_t*>(packet.buffer.firstElement->payload), packet.buffer.firstElement->len);
        }
    }else{
        // Pass to addressed one only
        ParticipantId_t id = getParticipantIdFromUnicastPort(packet.destPort, isUserPort(packet.destPort));
        if(id != PARTICIPANT_ID_INVALID){
#if DOMAIN_VERBOSE
            printf("Domain: Got unicast message on port %u\n", packet.destPort);
#endif
            if(id < m_nextParticipantId) {
                m_participants[id - PARTICIPANT_START_ID].newMessage(
                        static_cast<uint8_t *>(packet.buffer.firstElement->payload),
                        packet.buffer.firstElement->len);
            }else{
#if DOMAIN_VERBOSE
                printf("Domain: Participant id too high.\n");
#endif
            }
        }else{
#if DOMAIN_VERBOSE
            printf("Domain: Got message to port %u: no matching participant\n", packet.destPort);
#endif
        }
    }
}

rtps::Participant* Domain::createParticipant(){
#if DOMAIN_VERBOSE
    printf("Domain: Creating new participant.\n");
#endif
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
    spdpReader.m_attributes.endpointGuid = {part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER};

    // SEDP
    StatefullReader& sedpPubReader = m_statefullReaders[m_numStatefullReaders++];
    StatefullReader& sedpSubReader = m_statefullReaders[m_numStatefullReaders++];
    StatefullWriter& sedpPubWriter = m_statefullWriters[m_numStatefullWriters++];
    StatefullWriter& sedpSubWriter = m_statefullWriters[m_numStatefullWriters++];

    // Prepare attributes
    BuiltInTopicData sedpAttributes;
    sedpAttributes.topicName[0] = '\0';
    sedpAttributes.typeName[0] = '\0';
    sedpAttributes.reliabilityKind = ReliabilityKind_t::RELIABLE;
    sedpAttributes.endpointGuid.prefix = part.m_guidPrefix;
    sedpAttributes.unicastLocator = getBuiltInUnicastLocator(part.m_participantId);

    // READER
    sedpAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
    sedpPubReader.init(sedpAttributes, m_transport);
    sedpAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
    sedpSubReader.init(sedpAttributes, m_transport);

    // WRITER
    sedpAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
    sedpPubWriter.init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

    sedpAttributes.endpointGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
    sedpSubWriter.init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

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
    m_transport.createUdpConnection(getUserUnicastPort(part.m_participantId));
    m_transport.createUdpConnection(getBuiltInUnicastPort(part.m_participantId));
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


rtps::Reader* Domain::createReader(Participant& part, char* topicName, char* typeName, bool reliable){
    // TODO Distinguish WithKey and NoKey (Also changes EntityKind)
    BuiltInTopicData attributes;

    if(strlen(topicName) > Config::MAX_TOPICNAME_LENGTH || strlen(typeName) > Config::MAX_TYPENAME_LENGTH){
        return nullptr;
    }
    strcpy(attributes.topicName, topicName);
    strcpy(attributes.typeName, typeName);
    attributes.endpointGuid.prefix = part.m_guidPrefix;
    attributes.endpointGuid.entityId = {part.getNextUserEntityKey(), EntityKind_t::USER_DEFINED_READER_WITHOUT_KEY};
    attributes.unicastLocator = getUserUnicastLocator(part.m_participantId);

    if(reliable){
        if(m_numStatefullReaders == m_statefullReaders.size()){
            return nullptr;
        }

        attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

        StatefullReader& reader = m_statefullReaders[m_numStatefullReaders];
        reader.init(attributes, m_transport);

        part.addReader(&reader);
        return &reader;
    }else{
        if(m_numStatelessReaders == m_statelessReaders.size()){
            return nullptr;
        }

        attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

        StatelessReader& reader = m_statelessReaders[m_numStatelessReaders];
        reader.init(attributes);

        part.addReader(&reader);
        return &reader;
    }
}

rtps::GuidPrefix_t Domain::generateGuidPrefix(ParticipantId_t id) const{
    // TODO
    GuidPrefix_t prefix{1,2,3,4,5,6,7,8,9,10,11, *reinterpret_cast<uint8_t*>(&id)};
    return prefix;
}

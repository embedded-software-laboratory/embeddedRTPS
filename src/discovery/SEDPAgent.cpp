/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/Writer.h"
#include "rtps/entities/Participant.h"
#include "rtps/discovery/BuiltInTopicData.h"
#include "rtps/messages/MessageTypes.h"
#include "ucdr/microcdr.h"

using rtps::SEDPAgent;

void SEDPAgent::init(Participant& part, BuiltInEndpoints endpoints){
    m_part = &part;
    // TODO move
    if(sys_mutex_new(&m_mutex) != ERR_OK){
        printf("SEDPAgent failed to create mutex\n");
        return;
    }

    m_endpoints = endpoints;
    endpoints.sedpPubReader->registerCallback(receiveCallbackPublisher, this);
    endpoints.sedpSubReader->registerCallback(receiveCallbackSubscriber, this);
}

void SEDPAgent::receiveCallbackPublisher(void* callee, ReaderCacheChange& cacheChange){
    auto agent = static_cast<SEDPAgent*>(callee);
    agent->onNewPublisher(cacheChange);
}

void SEDPAgent::receiveCallbackSubscriber(void* callee, ReaderCacheChange& cacheChange){
    auto agent = static_cast<SEDPAgent*>(callee);
    agent->onNewSubscriber(cacheChange);
}

void SEDPAgent::onNewPublisher(ReaderCacheChange & /*change*/){
    // For now, reader don't care about matches
    return;
}

void SEDPAgent::onNewSubscriber(ReaderCacheChange &change){
    Lock lock{m_mutex};
    printf("New subscriber");

    if(!change.copyInto(m_buffer, sizeof(m_buffer)/sizeof(m_buffer[0]))){
        printf("SEDPAgent: Buffer too small.");
        return;
    }
    ucdrBuffer cdrBuffer;
    ucdr_init_buffer(&cdrBuffer, m_buffer, change.size);

    BuiltInTopicData topicData;
    if(topicData.readFromUcdrBuffer(cdrBuffer)){
        Writer* writer = m_part->getWriter(topicData.topicName, topicData.typeName);
        // TODO check policies
        printf("Found a new ");
        if(topicData.reliabilityKind == ReliabilityKind_t::RELIABLE){
            printf("reliable ");
        }else{
            printf("best-effort ");
        }
        printf("Subscriber\n");
        writer->addNewMatchedReader(ReaderProxy{topicData.endpointGuid, topicData.unicastLocator});
    }
}

void SEDPAgent::addWriter(Writer& writer){
    EntityKind_t writerKind = writer.m_attributes.endpointGuid.entityId.entityKind;
    if(writerKind == EntityKind_t::BUILD_IN_WRITER_WITH_KEY || writerKind == EntityKind_t::BUILD_IN_WRITER_WITHOUT_KEY){
        return; // No need to announce builtin endpoints
    }

    Lock lock{m_mutex};
    ucdrBuffer microbuffer;
    ucdr_init_buffer(&microbuffer, m_buffer, sizeof(m_buffer)/sizeof(m_buffer[0]));
    writer.m_attributes.serializeIntoUcdrBuffer(microbuffer);
    m_endpoints.sedpPubWriter->newChange(ChangeKind_t::ALIVE, m_buffer, ucdr_buffer_length(&microbuffer));
    printf("Added new change to sedpPubWriter.\n");
}

void SEDPAgent::addReader(Reader& reader){

}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/Writer.h"
#include "rtps/entities/Participant.h"
#include "rtps/discovery/TopicData.h"
#include "rtps/messages/MessageTypes.h"
#include "ucdr/microcdr.h"

#define SEDP_VERBOSE 0

using rtps::SEDPAgent;

void SEDPAgent::init(Participant& part, BuiltInEndpoints endpoints){
    m_part = &part;
    // TODO move
    if(sys_mutex_new(&m_mutex) != ERR_OK){
#if SEDP_VERBOSE
        printf("SEDPAgent failed to create mutex\n");
#endif
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

void SEDPAgent::onNewPublisher(ReaderCacheChange& change){
    //Lock lock{m_mutex};
#if SEDP_VERBOSE
    printf("New publisher\n");
#endif

    if(!change.copyInto(m_buffer, sizeof(m_buffer)/sizeof(m_buffer[0]))){
#if SEDP_VERBOSE
        printf("SEDPAgent: Buffer too small.\n");
#endif
        return;
    }
    ucdrBuffer cdrBuffer;
    ucdr_init_buffer(&cdrBuffer, m_buffer, change.size);

    TopicData topicData;
    if(topicData.readFromUcdrBuffer(cdrBuffer)){
        Reader* reader = m_part->getMatchingReader(topicData);
        if(reader == nullptr){
#if SEDP_VERBOSE
            printf("SEDPAgent: Couldn't find reader for new Publisher[%s, %s]\n", topicData.topicName, topicData.typeName);
#endif
            return;
        }
        // TODO check policies
#if SEDP_VERBOSE
        printf("Found a new ");
        if(topicData.reliabilityKind == ReliabilityKind_t::RELIABLE){
            printf("reliable ");
        }else{
            printf("best-effort ");
        }
        printf("publisher\n");
#endif
        reader->addNewMatchedWriter(WriterProxy{topicData.endpointGuid, topicData.unicastLocator});
    }
}

void SEDPAgent::onNewSubscriber(ReaderCacheChange& change){
    //Lock lock{m_mutex};
#if SEDP_VERBOSE
    printf("New subscriber\n");
#endif

    if(!change.copyInto(m_buffer, sizeof(m_buffer)/sizeof(m_buffer[0]))){
#if SEDP_VERBOSE
        printf("SEDPAgent: Buffer too small.");
#endif
        return;
    }
    ucdrBuffer cdrBuffer;
    ucdr_init_buffer(&cdrBuffer, m_buffer, change.size);

    TopicData topicData;
    if(topicData.readFromUcdrBuffer(cdrBuffer)){
        Writer* writer = m_part->getMatchingWriter(topicData);
        if(writer == nullptr) {
#if SEDP_VERBOSE
            printf("SEDPAgent: Couldn't find writer for new subscriber[%s, %s]\n", topicData.topicName, topicData.typeName);
#endif
            return;
        }

        // TODO check policies
#if SEDP_VERBOSE
        printf("Found a new ");
        if(topicData.reliabilityKind == ReliabilityKind_t::RELIABLE){
            printf("reliable ");
        }else{
            printf("best-effort ");
        }
        printf("Subscriber\n");
#endif
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
    const uint16_t zero_options = 0;

    ucdr_serialize_array_uint8_t(&microbuffer, rtps::SMElement::SCHEME_PL_CDR_LE.data(), rtps::SMElement::SCHEME_PL_CDR_LE.size());
    ucdr_serialize_uint16_t(&microbuffer, zero_options);
    writer.m_attributes.serializeIntoUcdrBuffer(microbuffer);
    m_endpoints.sedpPubWriter->newChange(ChangeKind_t::ALIVE, m_buffer, ucdr_buffer_length(&microbuffer));
#if SEDP_VERBOSE
    printf("Added new change to sedpPubWriter.\n");
#endif
}

void SEDPAgent::addReader(Reader& reader){
    EntityKind_t readerKind = reader.m_attributes.endpointGuid.entityId.entityKind;
    if(readerKind == EntityKind_t::BUILD_IN_READER_WITH_KEY || readerKind == EntityKind_t::BUILD_IN_READER_WITHOUT_KEY){
        return; // No need to announce builtin endpoints
    }

    Lock lock{m_mutex};
    ucdrBuffer microbuffer;
    ucdr_init_buffer(&microbuffer, m_buffer, sizeof(m_buffer)/sizeof(m_buffer[0]));
    const uint16_t zero_options = 0;

    ucdr_serialize_array_uint8_t(&microbuffer, rtps::SMElement::SCHEME_PL_CDR_LE.data(), rtps::SMElement::SCHEME_PL_CDR_LE.size());
    ucdr_serialize_uint16_t(&microbuffer, zero_options);
    reader.m_attributes.serializeIntoUcdrBuffer(microbuffer);
    m_endpoints.sedpSubWriter->newChange(ChangeKind_t::ALIVE, m_buffer, ucdr_buffer_length(&microbuffer));

#if SEDP_VERBOSE
    printf("Added new change to sedpSubWriter.\n");
#endif
}

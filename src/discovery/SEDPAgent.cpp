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

using rtps::SEDPAgent;

void SEDPAgent::init(Participant* part, BuiltInEndpoints endpoints){
    m_part = part;
    // TODO move
    if(sys_mutex_new(&m_mutex) != ERR_OK){
        printf("SEDPAgent failed to create mutex\n");
        return;
    }
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
        writer->addNewMatchedReader(ReaderProxy{topicData.endpointGuid, topicData.unicastLocator});
    }
}

void SEDPAgent::addWriter(Writer& writer){

}

void SEDPAgent::addReader(Reader& reader){

}
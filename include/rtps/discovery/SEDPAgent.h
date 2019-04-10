/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SEDPAGENT_H
#define RTPS_SEDPAGENT_H

#include "rtps/discovery/TopicData.h"
#include "rtps/discovery/BuiltInEndpoints.h"

namespace rtps{

    class Participant;
    class ReaderCacheChange;
    class Writer;
    class Reader;

    class SEDPAgent{
    public:
        void init(Participant& part, const BuiltInEndpoints& endpoints);
        void addWriter(Writer& writer);
        void addReader(Reader& reader);
        void registerOnNewPublisherMatchedCallback(void (*callback)(void* arg), void* args);
        void registerOnNewSubscriberMatchedCallback(void (*callback)(void* arg), void* args);

    protected: // For testing purposes
        void onNewPublisher(const TopicData& writerData);
        void onNewSubscriber(const TopicData& writerData);

    private:
        Participant* m_part;
        sys_mutex_t m_mutex;
        uint8_t m_buffer[300]; // TODO check size
        BuiltInEndpoints m_endpoints;
        void (*mfp_onNewPublisherCallback)(void* arg) = nullptr;
        void* m_onNewPublisherArgs = nullptr;
        void (*mfp_onNewSubscriberCallback)(void* arg) = nullptr;
        void* m_onNewSubscriberArgs = nullptr;

        static void receiveCallbackPublisher(void* callee, const ReaderCacheChange& cacheChange);
        static void receiveCallbackSubscriber(void* callee, const ReaderCacheChange& cacheChange);
        void onNewPublisher(const ReaderCacheChange& change);
        void onNewSubscriber(const ReaderCacheChange& change);

    };
}

#endif //RTPS_SEDPAGENT_H

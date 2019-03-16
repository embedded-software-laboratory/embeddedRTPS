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
        void init(Participant& part, BuiltInEndpoints endpoints);
        void addWriter(Writer& writer);
        void addReader(Reader& reader);

    private:
        Participant* m_part;
        sys_mutex_t m_mutex;
        uint8_t m_buffer[300];
        BuiltInEndpoints m_endpoints;

        static void receiveCallbackPublisher(void* callee, ReaderCacheChange& cacheChange);
        static void receiveCallbackSubscriber(void* callee, ReaderCacheChange& cacheChange);
        void onNewPublisher(ReaderCacheChange &change);
        void onNewSubscriber(ReaderCacheChange &change);

    };
}

#endif //RTPS_SEDPAGENT_H

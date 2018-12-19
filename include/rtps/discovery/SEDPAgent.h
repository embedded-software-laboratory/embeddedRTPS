/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SEDPAGENT_H
#define RTPS_SEDPAGENT_H

#include "rtps/messages/MessageReceiver.h"
#include "rtps/discovery/BuiltInTopicData.h"

namespace rtps{

    class Participant;
    class ReaderCacheChange;

    class SEDPAgent{
    public:
        void init(Participant* part, BuiltInEndpoints endpoints);

    private:
        static void receiveCallbackPublisher(void* callee, ReaderCacheChange& cacheChange);
        static void receiveCallbackSubscriber(void* callee, ReaderCacheChange& cacheChange);
        void newPublisher(ReaderCacheChange& change);
        void newSubscriber(ReaderCacheChange& change);

        Participant* m_part;
        sys_mutex_t m_mutex;
        uint8_t m_buffer[300];

    };
}

#endif //RTPS_SEDPAGENT_H

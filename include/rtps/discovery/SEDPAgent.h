/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_SEDPAGENT_H
#define RTPS_SEDPAGENT_H

#include <rtps/messages/MessageReceiver.h>

namespace rtps{

    class ReaderCacheChange;

    class SEDPAgent{
    public:
        void init(BuiltInEndpoints endpoints);

    private:
        static void receiveCallback(void* callee, ReaderCacheChange& cacheChange);
    };
}

#endif //RTPS_SEDPAGENT_H

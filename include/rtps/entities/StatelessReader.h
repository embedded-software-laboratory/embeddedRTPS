/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATELESSREADER_H
#define RTPS_STATELESSREADER_H

#include "rtps/entities/Reader.h"
#include "rtps/storages/HistoryCache.h"

namespace rtps{
    class StatelessReader final: public Reader{
    public:
        void init(const BuiltInTopicData& attributes);
        void newChange(ReaderCacheChange& cacheChange) override;
        void registerCallback(ddsReaderCallback_fp cb, void* callee) override;
        bool onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix) override;
    private:
        ddsReaderCallback_fp m_callback = nullptr;
        void* m_callee = nullptr;
    };

}

#endif //RTPS_STATELESSREADER_H

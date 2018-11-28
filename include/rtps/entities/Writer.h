/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_WRITER_H
#define RTPS_WRITER_H

#include "rtps/ThreadPool.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/HistoryCache.h"

namespace rtps{

    class Writer{
    public:
        virtual void createMessageCallback(ThreadPool::PacketInfo& buffer) = 0;
        virtual const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size) = 0;
        virtual void unsentChangesReset() = 0;

    protected:
        virtual ~Writer() = default;
    };
}

#endif //RTPS_WRITER_H

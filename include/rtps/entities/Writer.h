/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_WRITER_H
#define RTPS_WRITER_H

#include "rtps/ThreadPool.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/HistoryCache.h"
#include "rtps/entities/ReaderLocator.h"

namespace rtps{

    class Writer{
    public:
        virtual bool addNewMatchedReader(ReaderLocator loc) = 0;

        //! Executes required steps like sending packets. Intended to be called by worker threads
        virtual void progress() = 0;
        virtual const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) = 0;
        virtual void unsentChangesReset() = 0;

    protected:
        virtual ~Writer() = default;
    };
}

#endif //RTPS_WRITER_H

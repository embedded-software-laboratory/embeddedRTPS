/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/common/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{

    typedef void (*ddsReaderCallback_fp)(void* callee, ChangeKind_t kind, const uint8_t* data, DataSize_t size);

    class Reader{
    public:
        EntityId_t entityId = ENTITYID_UNKNOWN;
        virtual void newChange(ChangeKind_t kind, const uint8_t*, rtps::DataSize_t) = 0;
        virtual void registerCallback(ddsReaderCallback_fp cb, void* callee) = 0;
    protected:
        virtual ~Reader() = default;
    };
}

#endif //RTPS_READER_H

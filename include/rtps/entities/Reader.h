/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{

    typedef void (*ddsReaderCallback_fp)(const uint8_t* data, data_size_t size);

    class Reader{
    public:
        EntityId_t entityId = ENTITYID_UNKNOWN;
        virtual void newChange(ChangeKind_t kind, const uint8_t*, rtps::data_size_t) = 0;
        virtual void registerCallback(ddsReaderCallback_fp cb) = 0;
    protected:
        virtual ~Reader() = default;
    };
}

#endif //RTPS_READER_H

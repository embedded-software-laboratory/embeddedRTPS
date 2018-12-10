/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/common/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{

    class ReaderCacheChange{
    private:
        const uint8_t* data;

    public:
        const ChangeKind_t kind;
        const DataSize_t size;
        const Guid writerGuid;
        const SequenceNumber_t sn;

        ReaderCacheChange(ChangeKind_t kind, Guid& writerGuid, SequenceNumber_t sn, const uint8_t* data, DataSize_t size)
            : data(data), kind(kind), size(size), writerGuid(writerGuid), sn(sn){};

        bool copyInto(uint8_t* buffer, DataSize_t destSize){
            if(destSize < size){
                return false;
            }else{
                memcpy(buffer, data, size);
                return true;
            }
        }
    };

    typedef void (*ddsReaderCallback_fp)(void* callee, ReaderCacheChange& cacheChange);

    class Reader{
    public:
        EntityId_t entityId = ENTITYID_UNKNOWN;
        virtual void newChange(ReaderCacheChange& cacheChange) = 0;
        virtual void registerCallback(ddsReaderCallback_fp cb, void* callee) = 0;
    protected:
        virtual ~Reader() = default;
    };
}

#endif //RTPS_READER_H

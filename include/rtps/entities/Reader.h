/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/TopicData.h"
#include "rtps/entities/WriterProxy.h"
#include "rtps/storages/PBufWrapper.h"
#include <cstring>

namespace rtps{

    struct SubmessageHeartbeat;

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

        ~ReaderCacheChange() = default; // No need to free data. It's not owned by this object
        // Not allowed because this class doesn't own the ptr and the user isn't allowed to use it outside the
        // Scope of the callback
        ReaderCacheChange(const ReaderCacheChange& other) = delete;
        ReaderCacheChange(ReaderCacheChange&& other) = delete;
        ReaderCacheChange& operator=(const ReaderCacheChange &other) = delete;
        ReaderCacheChange& operator=(ReaderCacheChange&& other) = delete;


        bool copyInto(uint8_t* buffer, DataSize_t destSize) const{
            if(destSize < size){
                return false;
            }else{
                memcpy(buffer, data, size);
                return true;
            }
        }
    };

    typedef void (*ddsReaderCallback_fp)(void* callee, const ReaderCacheChange& cacheChange);

    class Reader{
    public:
        TopicData m_attributes;
        virtual void newChange(const ReaderCacheChange& cacheChange) = 0;
        virtual void registerCallback(ddsReaderCallback_fp cb, void* callee) = 0;
        virtual bool onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix) = 0;
        virtual bool addNewMatchedWriter(const WriterProxy& newProxy) = 0;
        virtual void removeWriter(const Guid& guid) = 0;
    protected:
        virtual ~Reader() = default;
    };
}

#endif //RTPS_READER_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{
    class Reader{
    public:
        const EntityId_t entityId;
        virtual void newChange(ChangeKind_t kind, const uint8_t*, rtps::data_size_t) = 0;
        virtual void registerCallback() = 0;
    protected:
        explicit Reader(EntityId_t id) : entityId(id){

        };

        virtual ~Reader() = default;
    };
}

#endif //RTPS_READER_H

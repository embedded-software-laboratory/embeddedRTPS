/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READERLOCATOR_H
#define RTPS_READERLOCATOR_H

#include "rtps/common/types.h"
#include "rtps/common/Locator.h"

namespace rtps{

    struct ReaderLocator{
        ReaderLocator() : entityId(ENTITYID_UNKNOWN), locator(){

        }
        ReaderLocator(EntityId_t id, Locator loc) : entityId(id), locator(loc){

        }
        EntityId_t entityId;
        Locator locator;
    };

} // rtps
#endif //RTPS_READERLOCATOR_H

//
// Created by cpmnuc1 on 16.01.19.
//

#ifndef RTPS_GLOBALS_H
#define RTPS_GLOBALS_H

#include "rtps/common/types.h"
#include "rtps/utils/udpUtils.h"

namespace TestGlobals{
    const rtps::GuidPrefix_t someGuidPrefix{1,2,3,4,5,6,7,8,9,10,11};
    const rtps::EntityId_t someWriterId{{1,2,3}, rtps::EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY};
    const rtps::Guid someWriterGuid{{1,2,3,4,5,6,7,8,9,10,11}, someWriterId};
    const rtps::EntityId_t someReaderId{{1,2,5}, rtps::EntityKind_t::USER_DEFINED_READER_WITHOUT_KEY};
    const rtps::Guid someReaderGuid{{1,2,3,4,5,6,7,8,9,10,11}, someReaderId};

    const rtps::Locator someUserUnicastLocator = rtps::Locator::createUDPv4Locator(192,168,0,88, rtps::getUserUnicastPort(0));
}

#endif //RTPS_GLOBALS_H

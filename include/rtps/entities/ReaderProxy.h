/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READERPROXY_H
#define RTPS_READERPROXY_H

#include "rtps/common/types.h"
#include "rtps/discovery/ParticipantProxyData.h"

namespace rtps{
    struct ReaderProxy{
        Guid remoteReaderGuid;
        Locator remoteLocator;
    };
}

#endif //RTPS_READERPROXY_H

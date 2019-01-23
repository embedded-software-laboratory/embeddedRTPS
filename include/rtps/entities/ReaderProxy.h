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
        SequenceNumberSet ackNackSet;
        Count_t ackNackCount;

        ReaderProxy() : remoteReaderGuid({GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}){};
        ReaderProxy(const Guid& guid, const Locator& loc)
            : remoteReaderGuid(guid), remoteLocator(loc),
              ackNackSet(), ackNackCount{0}{};

    };
}

#endif //RTPS_READERPROXY_H

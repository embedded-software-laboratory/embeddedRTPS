/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_WRITERPROXY_H
#define RTPS_WRITERPROXY_H

#include "rtps/common/types.h"
#include "rtps/common/Locator.h"

namespace rtps{
    struct WriterProxy{
        Guid remoteWriterGuid;
        SequenceNumber_t expectedSN;
        Count_t ackNackCount;
        Locator locator;

        void init(Guid& guid){
            remoteWriterGuid = guid;
            expectedSN = SequenceNumber_t{0,0};
            ackNackCount.value = 0;
        }

        SequenceNumberSet getMissing(const SequenceNumber_t& /*firstAvail*/, const SequenceNumber_t& /*lastAvail*/){
            SequenceNumberSet set;
            set.base = expectedSN;
            for(uint8_t bucket=0; bucket < set.bitMap.size(); ++bucket){
                set.bitMap[bucket] = ~static_cast<uint32_t>(0);
            }
            return set;
        }

        Count_t getNextCount(){
            const Count_t tmp = ackNackCount;
            ++ackNackCount.value;
            return tmp;
        }

    };
}

#endif //RTPS_WRITERPROXY_H

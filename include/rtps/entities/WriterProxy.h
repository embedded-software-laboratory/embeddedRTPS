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
        Count_t hbCount;
        Locator locator;

        WriterProxy(){};

        WriterProxy(const Guid& guid, const Locator& loc)
            : remoteWriterGuid(guid), expectedSN(SequenceNumber_t{0,1}), ackNackCount{1},
                hbCount{0}, locator(loc){
        }

        // For now, we don't store any packets, so we just request all starting from the next expected
        SequenceNumberSet getMissing(const SequenceNumber_t& /*firstAvail*/, const SequenceNumber_t& /*lastAvail*/){
            SequenceNumberSet set;
            set.numBits = 1;
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

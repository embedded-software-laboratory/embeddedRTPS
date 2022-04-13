/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#ifndef RTPS_WRITERPROXY_H
#define RTPS_WRITERPROXY_H

#include "rtps/common/types.h"
#include <rtps/common/Locator.h>

namespace rtps {
struct WriterProxy {
  Guid_t remoteWriterGuid;
  SequenceNumber_t expectedSN;
  Count_t ackNackCount;
  Count_t hbCount;
  LocatorIPv4 remoteLocator;

  WriterProxy() = default;

  WriterProxy(const Guid_t &guid, const LocatorIPv4 &loc)
      : remoteWriterGuid(guid),
        expectedSN(SequenceNumber_t{0, 1}), ackNackCount{1}, hbCount{0},
        remoteLocator(loc) {}

  // For now, we don't store any packets, so we just request all starting from
  // the next expected
  SequenceNumberSet getMissing(const SequenceNumber_t & /*firstAvail*/,
                               const SequenceNumber_t &lastAvail) {
    SequenceNumberSet set;
    if (lastAvail < expectedSN) {
      set.base = expectedSN;
      set.numBits = 0;
    } else {
      set.numBits = 1;
      set.base = expectedSN;
      set.bitMap[0] = uint32_t{1} << 31;
    }

    return set;
  }

  Count_t getNextAckNackCount() {
    const Count_t tmp = ackNackCount;
    ++ackNackCount.value;
    return tmp;
  }
};
} // namespace rtps

#endif // RTPS_WRITERPROXY_H

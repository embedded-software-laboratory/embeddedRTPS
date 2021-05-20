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

#ifndef RTPS_DISCOVEREDWRITERDATA_H
#define RTPS_DISCOVEREDWRITERDATA_H

#define SUPPRESS_UNICAST 0

#include "rtps/common/Locator.h"
#include "rtps/config.h"
#include "ucdr/microcdr.h"
#include <array>

namespace rtps {

struct BuiltInTopicKey {
  std::array<uint32_t, 3> value;
};

struct TopicData {
  Guid_t endpointGuid;
  char typeName[Config::MAX_TYPENAME_LENGTH];
  char topicName[Config::MAX_TOPICNAME_LENGTH];
  ReliabilityKind_t reliabilityKind;
  DurabilityKind_t durabilityKind;
  Locator unicastLocator;
  Locator multicastLocator;

  TopicData()
      : endpointGuid(GUID_UNKNOWN), typeName{'\0'}, topicName{'\0'},
        reliabilityKind(ReliabilityKind_t::BEST_EFFORT),
        durabilityKind(DurabilityKind_t::TRANSIENT_LOCAL) {
    rtps::Locator someLocator = rtps::Locator::createUDPv4Locator(
        192, 168, 0, 42, rtps::getUserUnicastPort(0));
    unicastLocator = someLocator;
    multicastLocator = Locator();
  };

  TopicData(Guid_t guid, ReliabilityKind_t reliability, Locator loc)
      : endpointGuid(guid), typeName{'\0'}, topicName{'\0'},
        reliabilityKind(reliability),
        durabilityKind(DurabilityKind_t::TRANSIENT_LOCAL), unicastLocator(loc) {
  }

  bool matchesTopicOf(const TopicData &other);

  bool readFromUcdrBuffer(ucdrBuffer &buffer);
  bool serializeIntoUcdrBuffer(ucdrBuffer &buffer) const;
};
} // namespace rtps

#endif // RTPS_DISCOVEREDWRITERDATA_H

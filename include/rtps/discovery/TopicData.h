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

#include "rtps/config.h"
#include "rtps/utils/hash.h"
#include "ucdr/microcdr.h"
#include <array>
#include <rtps/common/Locator.h>

namespace rtps {

struct BuiltInTopicKey {
  std::array<uint32_t, 3> value;
};

struct TopicData {
  Guid_t endpointGuid;
  std::array<char, Config::MAX_TYPENAME_LENGTH> typeName;
  std::array<char, Config::MAX_TOPICNAME_LENGTH> topicName;
  ReliabilityKind_t reliabilityKind;
  DurabilityKind_t durabilityKind;
  FullLengthLocator unicastLocator;
  FullLengthLocator multicastLocator;

  uint8_t statusInfo;
  bool statusInfoValid;
  // Use Case: Remotes communicates id of deleted endpoint through key_hash
  // parameter
  EntityId_t entityIdFromKeyHash;
  bool entityIdFromKeyHashValid;

  TopicData()
      : endpointGuid(GUID_UNKNOWN), typeName{'\0'}, topicName{'\0'},
        reliabilityKind(ReliabilityKind_t::BEST_EFFORT),
        durabilityKind(DurabilityKind_t::TRANSIENT_LOCAL) {
    rtps::FullLengthLocator someLocator =
        rtps::FullLengthLocator::createUDPv4Locator(
            192, 168, 0, 42, rtps::getUserUnicastPort(0));
    unicastLocator = someLocator;
    multicastLocator = FullLengthLocator();
  };

  TopicData(Guid_t guid, ReliabilityKind_t reliability, FullLengthLocator loc)
      : endpointGuid(guid), typeName{'\0'}, topicName{'\0'},
        reliabilityKind(reliability),
        durabilityKind(DurabilityKind_t::TRANSIENT_LOCAL), unicastLocator(loc) {
  }

  bool matchesTopicOf(const TopicData &other);

  bool readFromUcdrBuffer(ucdrBuffer &buffer);
  bool serializeIntoUcdrBuffer(ucdrBuffer &buffer) const;

  bool isDisposedFlagSet() const;
  bool isUnregisteredFlagSet() const;
};

struct TopicDataCompressed {
  Guid_t endpointGuid;
  std::size_t topicHash;
  std::size_t typeHash;
  bool is_reliable;
  LocatorIPv4 unicastLocator;
  LocatorIPv4 multicastLocator;

  TopicDataCompressed() = default;
  TopicDataCompressed(const TopicData &topic_data) {
    endpointGuid = topic_data.endpointGuid;
    topicHash =
        hashCharArray(topic_data.topicName.data(), topic_data.topicName.size());
    typeHash = hashCharArray(topic_data.typeName.data(),topic_data.typeName.size());
    is_reliable = (topic_data.reliabilityKind == ReliabilityKind_t::RELIABLE)
                      ? true
                      : false;
    unicastLocator = topic_data.unicastLocator;
    multicastLocator = topic_data.multicastLocator;
  }

  bool matchesTopicOf(const TopicData &topic_data) const;
};
} // namespace rtps

#endif // RTPS_DISCOVEREDWRITERDATA_H

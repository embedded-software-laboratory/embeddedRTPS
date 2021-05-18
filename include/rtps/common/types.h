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

#ifndef RTPS_TYPES_H
#define RTPS_TYPES_H

#include "lwip/ip4_addr.h"

#include <array>
#include <cstdint>
#include <initializer_list>

// TODO subnamespaces
namespace rtps {

// TODO move types to where they are needed!

typedef uint16_t Ip4Port_t;
typedef uint16_t DataSize_t;
typedef int8_t ParticipantId_t; // With UDP only 120 possible

enum class EntityKind_t : uint8_t {
  USER_DEFINED_UNKNOWN = 0x00,
  // No user define participant
  USER_DEFINED_WRITER_WITH_KEY = 0x02,
  USER_DEFINED_WRITER_WITHOUT_KEY = 0x03,
  USER_DEFINED_READER_WITHOUT_KEY = 0x04,
  USER_DEFINED_READER_WITH_KEY = 0x07,

  BUILD_IN_UNKNOWN = 0xc0,
  BUILD_IN_PARTICIPANT = 0xc1,
  BUILD_IN_WRITER_WITH_KEY = 0xc2,
  BUILD_IN_WRITER_WITHOUT_KEY = 0xc3,
  BUILD_IN_READER_WITHOUT_KEY = 0xc4,
  BUILD_IN_READER_WITH_KEY = 0xc7,

  VENDOR_SPEC_UNKNOWN = 0x40,
  VENDOR_SPEC_PARTICIPANT = 0x41,
  VENDOR_SPEC_WRITER_WITH_KEY = 0x42,
  VENDOR_SPEC_WRITER_WITHOUT_KEY = 0x43,
  VENDOR_SPEC_READER_WITHOUT_KEY = 0x44,
  VENDOR_SPEC_READER_WITH_KEY = 0x47
};

enum class TopicKind_t : uint8_t { NO_KEY = 1, WITH_KEY = 2 };

enum class ChangeKind_t : uint8_t {
  INVALID,
  ALIVE,
  NOT_ALIVE_DISPOSED,
  NOT_ALIVE_UNREGISTERED
};

enum class ReliabilityKind_t : uint32_t {
  BEST_EFFORT = 1,
  RELIABLE = 2 // Specification says 3 but eprosima sends 2
};

enum class DurabilityKind_t : uint32_t {
  VOLATILE = 0,
  TRANSIENT_LOCAL = 1,
  TRANSIENT = 2,
  PERSISTENT = 3
};

struct GuidPrefix_t {
  std::array<uint8_t, 12> id;

  bool operator==(const GuidPrefix_t &other) const {
    return this->id == other.id;
  }
};

struct EntityId_t {
  std::array<uint8_t, 3> entityKey;
  EntityKind_t entityKind;

  bool operator==(const EntityId_t &other) const {
    return this->entityKey == other.entityKey &&
           this->entityKind == other.entityKind;
  }

  bool operator!=(const EntityId_t &other) const { return !(*this == other); }
};

struct Guid_t {
  GuidPrefix_t prefix;
  EntityId_t entityId;

  bool operator==(const Guid_t &other) const {
    return this->prefix == other.prefix && this->entityId == other.entityId;
  }

  static uint32_t sum(const Guid_t &other) {
    uint32_t ret = 0;
    for (const auto &i : other.prefix.id) {
      ret += i;
    }
    for (const auto &i : other.entityId.entityKey) {
      ret += i;
    }
    return ret;
  }
};

// Described as long but there wasn't any definition. Other than 32 bit does not
// conform the default values
struct Time_t {
  int32_t seconds;   // time in seconds
  uint32_t fraction; // time in sec/2^32 (?)

  static Time_t create(int32_t s, uint32_t ns) {
    static constexpr double factor =
        (static_cast<uint64_t>(1) << 32) / 1000000000.;
    auto fraction = static_cast<uint32_t>(ns * factor);
    return Time_t{s, fraction};
  }
};

struct VendorId_t {
  std::array<uint8_t, 2> vendorId;
};

struct SequenceNumber_t {
  int32_t high;
  uint32_t low;

  bool operator==(const SequenceNumber_t &other) const {
    return high == other.high && low == other.low;
  }

  bool operator!=(const SequenceNumber_t &other) const {
    return !(*this == other);
  }

  bool operator<(const SequenceNumber_t &other) const {
    return high < other.high || (high == other.high && low < other.low);
  }

  bool operator<=(const SequenceNumber_t &other) const {
    return *this == other || *this < other;
  }

  SequenceNumber_t &operator++() {
    ++low;
    if (low == 0) {
      ++high;
    }
    return *this;
  }

  SequenceNumber_t operator++(int) {
    SequenceNumber_t tmp(*this);
    ++*this;
    return tmp;
  }
};

const uint32_t SNS_NUM_BITS = 32;
struct SequenceNumberSet {

  SequenceNumberSet() = default;
  explicit SequenceNumberSet(const SequenceNumber_t &firstMissing)
      : base(firstMissing) {}

  SequenceNumber_t base = {0, 0};
  // Cannot be static because of packed
  uint32_t numBits = SNS_NUM_BITS;
  std::array<uint32_t, 8> bitMap{};

  // We only need 1 byte because atm we don't store packets.
  bool isSet(uint32_t bit) const {
    if (bit >= SNS_NUM_BITS) {
      return true;
    }
    const auto bucket = static_cast<uint8_t>(bit / 32);
    const auto pos = static_cast<uint8_t>(bit % 32);
    return (bitMap[bucket] & (1 << (31 - pos))) != 0;
  }
};

struct FragmentNumber_t {
  uint32_t value;
};

struct Count_t {
  int32_t value;
};

struct ProtocolVersion_t {
  uint8_t major;
  uint8_t minor;
};

typedef Time_t Duration_t; // TODO

enum class ChangeForReaderStatusKind {
  UNSENT,
  UNACKNOWLEDGED,
  REQURESTED,
  ACKNOWLEDGED,
  UNDERWAY
};

enum class ChangeFromWriterStatusKind { LOST, MISSING, RECEIVED, UNKNOWN };

struct InstanceHandle_t { // TODO
  uint64_t value;
};

struct ParticipantMessageData { // TODO
};

/* Default Values */
const EntityId_t ENTITYID_UNKNOWN{};
const EntityId_t ENTITYID_BUILD_IN_PARTICIPANT = {
    {00, 00, 01}, EntityKind_t::BUILD_IN_PARTICIPANT};
const EntityId_t ENTITYID_SEDP_BUILTIN_TOPIC_WRITER = {
    {00, 00, 02}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
const EntityId_t ENTITYID_SEDP_BUILTIN_TOPIC_READER = {
    {00, 00, 02}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
const EntityId_t ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER = {
    {00, 00, 03}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
const EntityId_t ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER = {
    {00, 00, 03}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
const EntityId_t ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER = {
    {00, 00, 04}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
const EntityId_t ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER = {
    {00, 00, 04}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
const EntityId_t ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER = {
    {00, 01, 00}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
const EntityId_t ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER = {
    {00, 01, 00}, EntityKind_t::BUILD_IN_READER_WITH_KEY};
const EntityId_t ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER = {
    {00, 02, 00}, EntityKind_t::BUILD_IN_WRITER_WITH_KEY};
const EntityId_t ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER = {
    {00, 02, 00}, EntityKind_t::BUILD_IN_READER_WITH_KEY};

const GuidPrefix_t GUIDPREFIX_UNKNOWN{};
const Guid_t GUID_UNKNOWN{};

const ParticipantId_t PARTICIPANT_ID_INVALID = -1;

const ProtocolVersion_t PROTOCOLVERSION_1_0 = {1, 0};
const ProtocolVersion_t PROTOCOLVERSION_1_1 = {1, 1};
const ProtocolVersion_t PROTOCOLVERSION_2_0 = {2, 0};
const ProtocolVersion_t PROTOCOLVERSION_2_1 = {2, 1};
const ProtocolVersion_t PROTOCOLVERSION_2_2 = {2, 2};
const ProtocolVersion_t PROTOCOLVERSION = PROTOCOLVERSION_2_2;

const SequenceNumber_t SEQUENCENUMBER_UNKNOWN = {-1, 0};

const Time_t TIME_ZERO = {};
const Time_t TIME_INVALID = {-1, 0xFFFFFFFF};
const Time_t TIME_INFINITE = {0x7FFFFFFF, 0xFFFFFFFF};

const VendorId_t VENDOR_UNKNOWN = {};
} // namespace rtps

#endif // RTPS_TYPES_H

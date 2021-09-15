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

#ifndef RTPS_MESSAGES_H
#define RTPS_MESSAGES_H

#include "rtps/common/types.h"

#include <array>

namespace rtps {

namespace SMElement {
// TODO endianess
enum ParameterId : uint16_t {
  PID_PAD = 0x0000,
  PID_SENTINEL = 0x0001,
  PID_USER_DATA = 0x002c,
  PID_TOPIC_NAME = 0x0005,
  PID_TYPE_NAME = 0x0007,
  PID_GROUP_DATA = 0x002d,
  PID_TOPIC_DATA = 0x002e,
  PID_DURABILITY = 0x001d,
  PID_DURABILITY_SERVICE = 0x001e,
  PID_DEADLINE = 0x0023,
  PID_LATENCY_BUDGET = 0x0027,
  PID_LIVELINESS = 0x001b,
  PID_RELIABILITY = 0x001a,
  PID_LIFESPAN = 0x002b,
  PID_DESTINATION_ORDER = 0x0025,
  PID_HISTORY = 0x0040,
  PID_RESOURCE_LIMITS = 0x0041,
  PID_OWNERSHIP = 0x001f,
  PID_OWNERSHIP_STRENGTH = 0x0006,
  PID_PRESENTATION = 0x0021,
  PID_PARTITION = 0x0029,
  PID_TIME_BASED_FILTER = 0x0004,
  PID_TRANSPORT_PRIORITY = 0x0049,
  PID_PROTOCOL_VERSION = 0x0015,
  PID_VENDORID = 0x0016,
  PID_UNICAST_LOCATOR = 0x002f,
  PID_MULTICAST_LOCATOR = 0x0030,
  PID_MULTICAST_IPADDRESS = 0x0011,
  PID_DEFAULT_UNICAST_LOCATOR = 0x0031,
  PID_DEFAULT_MULTICAST_LOCATOR = 0x0048,
  PID_METATRAFFIC_UNICAST_LOCATOR = 0x0032,
  PID_METATRAFFIC_MULTICAST_LOCATOR = 0x0033,
  PID_DEFAULT_UNICAST_IPADDRESS = 0x000c,
  PID_DEFAULT_UNICAST_PORT = 0x000e,
  PID_METATRAFFIC_UNICAST_IPADDRESS = 0x0045,
  PID_METATRAFFIC_UNICAST_PORT = 0x000d,
  PID_METATRAFFIC_MULTICAST_IPADDRESS = 0x000b,
  PID_METATRAFFIC_MULTICAST_PORT = 0x0046,
  PID_EXPECTS_INLINE_QOS = 0x0043,
  PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT = 0x0034,
  PID_PARTICIPANT_BUILTIN_ENDPOINTS = 0x0044,
  PID_PARTICIPANT_LEASE_DURATION = 0x0002,
  PID_CONTENT_FILTER_PROPERTY = 0x0035,
  PID_PARTICIPANT_GUID = 0x0050,
  PID_PARTICIPANT_ENTITYID = 0x0051,
  PID_GROUP_GUID = 0x0052,
  PID_GROUP_ENTITYID = 0x0053,
  PID_BUILTIN_ENDPOINT_SET = 0x0058,
  PID_PROPERTY_LIST = 0x0059,
  PID_ENDPOINT_GUID = 0x005a,
  PID_TYPE_MAX_SIZE_SERIALIZED = 0x0060,
  PID_ENTITY_NAME = 0x0062,
  PID_KEY_HASH = 0x0070,
  PID_STATUS_INFO = 0x0071
};

enum BuildInEndpointSet : uint32_t {
  DISC_BIE_PARTICIPANT_ANNOUNCER = 1 << 0,
  DISC_BIE_PARTICIPANT_DETECTOR = 1 << 1,
  DISC_BIE_PUBLICATION_ANNOUNCER = 1 << 2,
  DISC_BIE_PUBLICATION_DETECTOR = 1 << 3,
  DISC_BIE_SUBSCRIPTION_ANNOUNCER = 1 << 4,
  DISC_BIE_SUBSCRIPTION_DETECTOR = 1 << 5,

  DISC_BIE_PARTICIPANT_PROXY_ANNOUNCER = 1 << 6,
  DISC_BIE_PARTICIPANT_PROXY_DETECTOR = 1 << 7,
  DISC_BIE_PARTICIPANT_STATE_ANNOUNCER = 1 << 8,
  DISC_BIE_PARTICIPANT_STATE_DETECTOR = 1 << 9,

  BIE_PARTICIPANT_MESSAGE_DATA_WRITER = 1 << 10,
  BIE_PARTICIPANT_MESSAGE_DATA_READER = 1 << 11,
};

// TODO endianess

const std::array<uint8_t, 2> SCHEME_CDR_LE{0x00, 0x01};
const std::array<uint8_t, 2> SCHEME_PL_CDR_LE{0x00, 0x03};

struct ParameterList_t {
  ParameterId pid;
  uint16_t length;
  // Values follow
} __attribute__((packed));
} // namespace SMElement

enum class SubmessageKind : uint8_t {
  PAD = 0x01,            /* Pad */
  ACKNACK = 0x06,        /* AckNack */
  HEARTBEAT = 0x07,      /* Heartbeat */
  GAP = 0x08,            /* Gap */
  INFO_TS = 0x09,        /* InfoTimestamp */
  INFO_SRC = 0x0c,       /* InfoSource */
  INFO_REPLY_IP4 = 0x0d, /* InfoReplyIp4 */
  INFO_DST = 0x0e,       /* InfoDestination */
  INFO_REPLY = 0x0f,     /* InfoReply */
  NACK_FRAG = 0x12,      /* NackFrag */
  HEARTBEAT_FRAG = 0x13, /* HeartbeatFrag */
  DATA = 0x15,           /* Data */
  DATA_FRAG = 0x16       /* DataFrag */
};

enum SubMessageFlag : uint8_t {
  FLAG_ENDIANESS = (1 << 0),
  FLAG_BIG_ENDIAN = (0 << 0),
  FLAG_LITTLE_ENDIAN = (1 << 0),
  FLAG_INVALIDATE = (1 << 1),
  FLAG_INLINE_QOS = (1 << 1),
  FLAG_NO_PAYLOAD = (0 << 3 | 0 << 2),
  FLAG_DATA_PAYLOAD = (0 << 3 | 1 << 2),
  FLAG_FINAL = (1 << 1),
  FLAG_HB_LIVELINESS = (1 << 2)
};

const std::array<uint8_t, 4> RTPS_PROTOCOL_NAME = {'R', 'T', 'P', 'S'};
struct Header {
  std::array<uint8_t, 4> protocolName;
  ProtocolVersion_t protocolVersion;
  VendorId_t vendorId;
  GuidPrefix_t guidPrefix;
  static constexpr uint16_t getRawSize() {
    return sizeof(std::array<uint8_t, 4>) + sizeof(ProtocolVersion_t) +
           sizeof(VendorId_t) + sizeof(GuidPrefix_t);
  }
};

struct SubmessageHeader {
  SubmessageKind submessageId;
  uint8_t flags;
  uint16_t octetsToNextHeader;
  static constexpr uint16_t getRawSize() {
    return sizeof(SubmessageKind) + sizeof(uint8_t) + sizeof(uint16_t);
  }
};

struct SubmessageData {
  SubmessageHeader header;
  uint16_t extraFlags;
  uint16_t octetsToInlineQos;
  EntityId_t readerId;
  EntityId_t writerId;
  SequenceNumber_t writerSN;
  static constexpr uint16_t getRawSize() {
    return SubmessageHeader::getRawSize() + sizeof(uint16_t) +
           sizeof(uint16_t) + (2 * 3 + 2 * 1) // EntityID
           + sizeof(SequenceNumber_t);
  }
};

struct SubmessageHeartbeat {
  SubmessageHeader header;
  EntityId_t readerId;
  EntityId_t writerId;
  SequenceNumber_t firstSN;
  SequenceNumber_t lastSN;
  Count_t count;
  static constexpr uint16_t getRawSize() {
    return SubmessageHeader::getRawSize() + (2 * 3 + 2 * 1) // EntityID
           + 2 * sizeof(SequenceNumber_t) + sizeof(Count_t);
  }
};

struct SubmessageInfoDST {
  SubmessageHeader header;
  GuidPrefix_t guidPrefix;

  static constexpr uint16_t getRawSize() {
    return SubmessageHeader::getRawSize() + (sizeof(guidPrefix));
  }
};

struct SubmessageAckNack {
  SubmessageHeader header;
  EntityId_t readerId;
  EntityId_t writerId;
  SequenceNumberSet readerSNState;
  Count_t count;
  static uint16_t getRawSize(const SequenceNumberSet &set) {
    uint16_t bitMapSize = 0;
    if (set.numBits != 0) {
      bitMapSize = 4 * ((set.numBits / 32) + 1);
    }
    return getRawSizeWithoutSNSet() + sizeof(SequenceNumber_t) +
           sizeof(uint32_t) + bitMapSize; // SequenceNumberSet
  }
  static uint16_t getRawSizeWithoutSNSet() {
    return SubmessageHeader::getRawSize() + (2 * 3 + 2 * 1) // EntityID
           + sizeof(Count_t);
  }
};

template <typename Buffer>
bool serializeMessage(Buffer &buffer, Header &header) {
  if (!buffer.reserve(Header::getRawSize())) {
    return false;
  }

  buffer.append(header.protocolName.data(), sizeof(std::array<uint8_t, 4>));
  buffer.append(reinterpret_cast<uint8_t *>(&header.protocolVersion),
                sizeof(ProtocolVersion_t));
  buffer.append(header.vendorId.vendorId.data(), sizeof(VendorId_t));
  buffer.append(header.guidPrefix.id.data(), sizeof(GuidPrefix_t));
  return true;
}

template <typename Buffer>
bool serializeMessage(Buffer &buffer, SubmessageHeader &header) {
  if (!buffer.reserve(Header::getRawSize())) {
    return false;
  }
  buffer.reserve(SubmessageHeader::getRawSize());

  buffer.append(reinterpret_cast<uint8_t *>(&header.submessageId),
                sizeof(SubmessageKind));
  buffer.append(&header.flags, sizeof(uint8_t));
  buffer.append(reinterpret_cast<uint8_t *>(&header.octetsToNextHeader),
                sizeof(uint16_t));
  return true;
}

template <typename Buffer>
bool serializeMessage(Buffer &buffer, SubmessageInfoDST &msg) {
  if (!buffer.reserve(SubmessageInfoDST::getRawSize())) {
    return false;
  }

  bool ret = serializeMessage(buffer, msg.header);
  if (!ret) {
    return false;
  }

  return buffer.append(msg.guidPrefix.id.data(), sizeof(GuidPrefix_t));
}

template <typename Buffer>
bool serializeMessage(Buffer &buffer, SubmessageData &msg) {
  if (!buffer.reserve(SubmessageData::getRawSize())) {
    return false;
  }

  serializeMessage(buffer, msg.header);

  buffer.append(reinterpret_cast<uint8_t *>(&msg.extraFlags), sizeof(uint16_t));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.octetsToInlineQos),
                sizeof(uint16_t));
  buffer.append(msg.readerId.entityKey.data(), msg.readerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(msg.writerId.entityKey.data(), msg.writerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.writerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.writerSN.high),
                sizeof(msg.writerSN.high));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.writerSN.low),
                sizeof(msg.writerSN.low));
  return true;
}

template <typename Buffer>
bool serializeMessage(Buffer &buffer, SubmessageHeartbeat &msg) {
  if (!buffer.reserve(SubmessageHeartbeat::getRawSize())) {
    return false;
  }

  serializeMessage(buffer, msg.header);

  buffer.append(msg.readerId.entityKey.data(), msg.readerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(msg.writerId.entityKey.data(), msg.writerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.writerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.firstSN.high),
                sizeof(msg.firstSN.high));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.firstSN.low),
                sizeof(msg.firstSN.low));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.lastSN.high),
                sizeof(msg.lastSN.high));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.lastSN.low),
                sizeof(msg.lastSN.low));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.count.value),
                sizeof(msg.count.value));
  return true;
}

template <typename Buffer>
bool serializeMessage(Buffer &buffer, SubmessageAckNack &msg) {
  if (!buffer.reserve(SubmessageAckNack::getRawSize(msg.readerSNState))) {
    return false;
  }

  serializeMessage(buffer, msg.header);

  buffer.append(msg.readerId.entityKey.data(), msg.readerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(msg.writerId.entityKey.data(), msg.writerId.entityKey.size());
  buffer.append(reinterpret_cast<uint8_t *>(&msg.writerId.entityKind),
                sizeof(EntityKind_t));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerSNState.base.high),
                sizeof(msg.readerSNState.base.high));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerSNState.base.low),
                sizeof(msg.readerSNState.base.low));
  buffer.append(reinterpret_cast<uint8_t *>(&msg.readerSNState.numBits),
                sizeof(uint32_t));
  if (msg.readerSNState.numBits != 0) {
    buffer.append(reinterpret_cast<uint8_t *>(msg.readerSNState.bitMap.data()),
                  4 * ((msg.readerSNState.numBits / 32) + 1));
  }
  buffer.append(reinterpret_cast<uint8_t *>(&msg.count.value),
                sizeof(msg.count.value));
  return true;
}

struct MessageProcessingInfo {
  MessageProcessingInfo(const uint8_t *data, DataSize_t size)
      : data(data), size(size) {}
  const uint8_t *data;
  const DataSize_t size;

  //! Offset to the next unprocessed byte
  DataSize_t nextPos = 0;

  inline const uint8_t *getPointerToCurrentPos() const {
    return &data[nextPos];
  }

  //! Returns the size of data which isn't processed yet
  inline DataSize_t getRemainingSize() const { return size - nextPos; }
};

bool deserializeMessage(const MessageProcessingInfo &info, Header &header);

bool deserializeMessage(const MessageProcessingInfo &info,
                        SubmessageHeader &header);

bool deserializeMessage(const MessageProcessingInfo &info, SubmessageData &msg);

bool deserializeMessage(const MessageProcessingInfo &info,
                        SubmessageHeartbeat &msg);

bool deserializeMessage(const MessageProcessingInfo &info,
                        SubmessageAckNack &msg);

} // namespace rtps

#endif // RTPS_MESSAGES_H

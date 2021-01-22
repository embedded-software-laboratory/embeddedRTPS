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

#include "rtps/messages/MessageReceiver.h"
#include <rtps/entities/Participant.h>

#include "rtps/entities/Reader.h"
#include "rtps/entities/Writer.h"
#include "rtps/messages/MessageTypes.h"

using rtps::MessageReceiver;

#define RECV_VERBOSE 0

#if RECV_VERBOSE
#include "rtps/utils/printutils.h"
#endif

MessageReceiver::MessageReceiver(Participant *part) : mp_part(part) {}

void MessageReceiver::resetState() {
  sourceGuidPrefix = GUIDPREFIX_UNKNOWN;
  sourceVersion = PROTOCOLVERSION;
  sourceVendor = VENDOR_UNKNOWN;
  haveTimeStamp = false;
}

bool MessageReceiver::processMessage(const uint8_t *data, DataSize_t size) {
  resetState();
  MessageProcessingInfo msgInfo(data, size);

  if (!processHeader(msgInfo)) {
    return false;
  }
  SubmessageHeader submsgHeader;
  while (msgInfo.nextPos < msgInfo.size) {
    if (!deserializeMessage(msgInfo, submsgHeader)) {
      return false;
    }
    processSubmessage(msgInfo, submsgHeader);
  }

  return true;
}

bool MessageReceiver::processHeader(MessageProcessingInfo &msgInfo) {
  Header header;
  if (!deserializeMessage(msgInfo, header)) {
    return false;
  }

  if (header.guidPrefix.id == mp_part->m_guidPrefix.id) {
#if RECV_VERBOSE
    printf("[MessageReceiver]: Received own message.\n");
#endif
    return false; // Don't process our own packet
  }

  if (header.protocolName != RTPS_PROTOCOL_NAME ||
      header.protocolVersion.major != PROTOCOLVERSION.major) {
    return false;
  }

  sourceGuidPrefix = header.guidPrefix;
  sourceVendor = header.vendorId;
  sourceVersion = header.protocolVersion;

  msgInfo.nextPos += Header::getRawSize();
  return true;
}

bool MessageReceiver::processSubmessage(MessageProcessingInfo &msgInfo,
                                        const SubmessageHeader &submsgHeader) {
  bool success = false;

  switch (submsgHeader.submessageId) {
  case SubmessageKind::ACKNACK:
#if RECV_VERBOSE
    printf("Processing AckNack submessage\n");
#endif
    success = processAckNackSubmessage(msgInfo);
    break;
  case SubmessageKind::DATA:
#if RECV_VERBOSE
    printf("Processing Data submessage\n");
#endif
    success = processDataSubmessage(msgInfo);
    break;
  case SubmessageKind::HEARTBEAT:
#if RECV_VERBOSE
    printf("Processing Heartbeat submessage\n");
#endif
    success = processHeartbeatSubmessage(msgInfo);
    break;
  case SubmessageKind::INFO_DST:
#if RECV_VERBOSE
    printf("Info_DST submessage not relevant.\n");
#endif
    success = true; // Not relevant
    break;
  case SubmessageKind::INFO_TS:
#if RECV_VERBOSE
    printf("Info_TS submessage not relevant.\n");
#endif
    success = true; // Not relevant now
    break;
  default:
#if RECV_VERBOSE
    printf("Submessage of type %u currently not supported. Skipping..\n",
           static_cast<uint8_t>(submsgHeader.submessageId));
#endif
    success = false;
  }
  msgInfo.nextPos +=
      submsgHeader.submessageLength + SubmessageHeader::getRawSize();
  return success;
}

bool MessageReceiver::processDataSubmessage(MessageProcessingInfo &msgInfo) {
  SubmessageData dataSubmsg;
  if (!deserializeMessage(msgInfo, dataSubmsg)) {
    return false;
  }

  const uint8_t *serializedData =
      msgInfo.getPointerToCurrentPos() + SubmessageData::getRawSize();
  const DataSize_t size =
      msgInfo.size - (msgInfo.nextPos + SubmessageData::getRawSize());

  // bool isLittleEndian = (submsgHeader->flags &
  // SubMessageFlag::FLAG_ENDIANESS); bool hasInlineQos = (submsgHeader->flags &
  // SubMessageFlag::FLAG_INLINE_QOS);

  Reader *reader = mp_part->getReader(dataSubmsg.readerId);
  if (reader != nullptr) {
    Guid writerGuid{sourceGuidPrefix, dataSubmsg.writerId};
    ReaderCacheChange change{ChangeKind_t::ALIVE, writerGuid,
                             dataSubmsg.writerSN, serializedData, size};
    reader->newChange(change);
  } else {
#if RECV_VERBOSE
    printf("Couldn't find a reader with id: ");
    printEntityId(dataSubmsg.readerId);
    printf("\n");
#endif
  }

  return true;
}

bool MessageReceiver::processHeartbeatSubmessage(
    MessageProcessingInfo &msgInfo) {
  SubmessageHeartbeat submsgHB;
  if (!deserializeMessage(msgInfo, submsgHB)) {
    return false;
  }

  Reader *reader = mp_part->getReader(submsgHB.readerId);
  if (reader != nullptr) {
    reader->onNewHeartbeat(submsgHB, sourceGuidPrefix);
    return true;
  } else {
    return false;
  }
}

bool MessageReceiver::processAckNackSubmessage(MessageProcessingInfo &msgInfo) {
  SubmessageAckNack submsgAckNack;
  if (!deserializeMessage(msgInfo, submsgAckNack)) {
    return false;
  }

  Writer *writer = mp_part->getWriter(submsgAckNack.writerId);
  if (writer != nullptr) {
    writer->onNewAckNack(submsgAckNack, sourceGuidPrefix);
    return true;
  } else {
    return false;
  }
}

#undef RECV_VERBOSE

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageTypes.h"
using namespace rtps;

void doCopyAndMoveOn(uint8_t* dst, const uint8_t*& src, size_t size){
    memcpy(dst, src, size);
    src += size;
}

void rtps::deserializeMessage(const MessageProcessingInfo& info, Header& header){
    const uint8_t* currentPos = info.getPointerToPos();
    doCopyAndMoveOn(header.protocolName.data(), currentPos, sizeof(std::array<uint8_t, 4>));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&header.protocolVersion), currentPos, sizeof(ProtocolVersion_t));
    doCopyAndMoveOn(header.vendorId.vendorId.data(), currentPos, header.vendorId.vendorId.size());
    doCopyAndMoveOn(header.guidPrefix.id.data(), currentPos, header.guidPrefix.id.size());
}

void rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageHeader& header){
    const uint8_t* currentPos = info.getPointerToPos();
    header.submessageId = static_cast<SubmessageKind>(*currentPos++);
    header.flags = *(currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&header.submessageLength), currentPos, sizeof(uint16_t));
}

void rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageData& msg){
    deserializeMessage(info, msg.header);
    const uint8_t* currentPos = info.getPointerToPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.extraFlags), currentPos, sizeof(uint16_t));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.octetsToInlineQos), currentPos, sizeof(uint16_t));
    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.writerSN.high), currentPos, sizeof(msg.writerSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.writerSN.low), currentPos, sizeof(msg.writerSN.low));
}

void rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageHeartbeat& msg){
    deserializeMessage(info, msg.header);
    const uint8_t* currentPos = info.getPointerToPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.firstSN.high), currentPos, sizeof(msg.firstSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.firstSN.low), currentPos, sizeof(msg.firstSN.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.lastSN.high), currentPos, sizeof(msg.lastSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.lastSN.low), currentPos, sizeof(msg.lastSN.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.count.value), currentPos, sizeof(msg.count.value));
}

void rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageAckNack& msg){
    deserializeMessage(info, msg.header);
    const uint8_t* currentPos = info.getPointerToPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.readerSNState.base.high), currentPos, sizeof(msg.readerSNState.base.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.readerSNState.base.low), currentPos, sizeof(msg.readerSNState.base.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.count.value), currentPos, sizeof(msg.count.value));
}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageTypes.h"
#include <cstring>

#include "TFT.h"

#include <stdio.h>
using namespace rtps;

void doCopyAndMoveOn(uint8_t* dst, const uint8_t*& src, size_t size){
    memcpy(dst, src, size);
    src += size;
}

bool rtps::deserializeMessage(const MessageProcessingInfo& info, Header& header){
    if(info.getRemainingSize() < Header::getRawSize()){
        return false;
    }

    const uint8_t* currentPos = info.getPointerToCurrentPos();
    doCopyAndMoveOn(header.protocolName.data(), currentPos, sizeof(std::array<uint8_t, 4>));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&header.protocolVersion), currentPos, sizeof(ProtocolVersion_t));
    doCopyAndMoveOn(header.vendorId.vendorId.data(), currentPos, header.vendorId.vendorId.size());
    doCopyAndMoveOn(header.guidPrefix.id.data(), currentPos, header.guidPrefix.id.size());
    return true;
}

bool rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageHeader& header){
    if(info.getRemainingSize() < SubmessageHeader::getRawSize()){
        return false;
    }

    const uint8_t* currentPos = info.getPointerToCurrentPos();
    header.submessageId = static_cast<SubmessageKind>(*currentPos++);
    header.flags = *(currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&header.submessageLength), currentPos, sizeof(uint16_t));
    return true;
}

bool rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageData& msg){
    if(info.getRemainingSize() < SubmessageHeader::getRawSize()){
        return false;
    }
    if(!deserializeMessage(info, msg.header)){
        return false;
    }

    // Check for length including data
    if(info.getRemainingSize() < SubmessageHeader::getRawSize() + msg.header.submessageLength){
        return false;
    }

    const uint8_t* currentPos = info.getPointerToCurrentPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.extraFlags), currentPos, sizeof(uint16_t));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.octetsToInlineQos), currentPos, sizeof(uint16_t));
    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.writerSN.high), currentPos, sizeof(msg.writerSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.writerSN.low), currentPos, sizeof(msg.writerSN.low));
    return true;
}


bool rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageHeartbeat& msg){
    if(info.getRemainingSize() < SubmessageHeartbeat::getRawSize()){
        return false;
    }
    if(!deserializeMessage(info, msg.header)){
        return false;
    }

    const uint8_t* currentPos = info.getPointerToCurrentPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.firstSN.high), currentPos, sizeof(msg.firstSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.firstSN.low), currentPos, sizeof(msg.firstSN.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.lastSN.high), currentPos, sizeof(msg.lastSN.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.lastSN.low), currentPos, sizeof(msg.lastSN.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.count.value), currentPos, sizeof(msg.count.value));
    return true;
}

bool rtps::deserializeMessage(const MessageProcessingInfo& info, SubmessageAckNack& msg){
    const DataSize_t remainingSizeAtBeginning = info.getRemainingSize();
    if(remainingSizeAtBeginning < SubmessageAckNack::getRawSizeWithoutSNSet()){ // Size of SequenceNumberSet unknown
        return false;
    }
    if(!deserializeMessage(info, msg.header)){
        return false;
    }

    const uint8_t* currentPos = info.getPointerToCurrentPos() + SubmessageHeader::getRawSize();

    doCopyAndMoveOn(msg.readerId.entityKey.data(), currentPos, msg.readerId.entityKey.size());
    msg.readerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(msg.writerId.entityKey.data(), currentPos, msg.writerId.entityKey.size());
    msg.writerId.entityKind = static_cast<EntityKind_t>(*currentPos++);
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.readerSNState.base.high), currentPos, sizeof(msg.readerSNState.base.high));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.readerSNState.base.low), currentPos, sizeof(msg.readerSNState.base.low));
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.readerSNState.numBits), currentPos, sizeof(uint32_t));

    // Now we can check for full size
    if(remainingSizeAtBeginning < SubmessageAckNack::getRawSize(msg.readerSNState)){
        return false;
    }

    if(msg.readerSNState.numBits != 0){
    	if(4*((msg.readerSNState.numBits / 32) + 1) > msg.readerSNState.bitMap.size()){
    		while(1);
    	}
    	doCopyAndMoveOn(reinterpret_cast<uint8_t*>(msg.readerSNState.bitMap.data()), currentPos, 4*((msg.readerSNState.numBits / 32) + 1));
    }
    doCopyAndMoveOn(reinterpret_cast<uint8_t*>(&msg.count.value), currentPos, sizeof(msg.count.value));
    return true;
}

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <rtps/entities/Participant.h>
#include "rtps/messages/MessageReceiver.h"

#include "rtps/entities/Reader.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Writer.h"

using rtps::MessageReceiver;

#define RECV_VERBOSE 1

MessageReceiver::MessageReceiver(Participant* part)
: mp_part(part){

}

void MessageReceiver::reset(){
    sourceGuidPrefix = GUIDPREFIX_UNKNOWN;
    sourceVersion = PROTOCOLVERSION;
    sourceVendor = VENDOR_UNKNOWN;
    haveTimeStamp = false;
}

bool MessageReceiver::processMessage(const uint8_t* data, DataSize_t size){

    MessageProcessingInfo msgInfo{data, size};

    if(!processHeader(msgInfo)){
        return false;
    }

    while(msgInfo.nextPos < msgInfo.size){
        processSubMessage(msgInfo);
    }

    return true;

}

bool MessageReceiver::processHeader(MessageProcessingInfo& msgInfo){
    if(msgInfo.size < sizeof(rtps::Header)){
        return false;
    }

    auto header = reinterpret_cast<const Header*>(msgInfo.getPointerToPos());

    if(header->guidPrefix.id == mp_part->m_guidPrefix.id){
        return false; // Don't process our own packet
    }

    if(header->protocolName != RTPS_PROTOCOL_NAME ||
       header->protocolVersion.major != PROTOCOLVERSION.major){
        return false;
    }

    sourceGuidPrefix = header->guidPrefix;
    sourceVendor = header->vendorId;
    sourceVersion = header->protocolVersion;

    msgInfo.nextPos+= sizeof(Header);
    return true;
}

bool MessageReceiver::processSubMessage(MessageProcessingInfo& msgInfo){

    auto submsgHeader = reinterpret_cast<const SubmessageHeader*>(msgInfo.getPointerToPos());

    bool success;
    switch(submsgHeader->submessageId){
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
#ifdef RECV_VERBOSE
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
            printf("Submessage of type %u currently not supported. Skipping..\n", static_cast<uint8_t>(submsgHeader->submessageId));
            success = true;
    }
    msgInfo.nextPos+= submsgHeader->submessageLength + sizeof(SubmessageHeader);
    return success;
}

bool MessageReceiver::processDataSubmessage(MessageProcessingInfo& msgInfo){
    auto submsgData = reinterpret_cast<const SubmessageData*>(msgInfo.getPointerToPos());
    const uint8_t* serializedData = msgInfo.getPointerToPos() + sizeof(SubmessageData);
    const DataSize_t size = msgInfo.size - (msgInfo.nextPos + sizeof(SubmessageData));

    //if(submsgHeader->submessageLength > msgInfo.size - msgInfo.nextPos){
    //    return false;
    //}
    // TODO We can do better than that
    //bool isLittleEndian = (submsgHeader->flags & SubMessageFlag::FLAG_ENDIANESS);
    //bool hasInlineQos = (submsgHeader->flags & SubMessageFlag::FLAG_INLINE_QOS);

    Reader* reader = mp_part->getReader(submsgData->readerId);
    if(reader != nullptr){
        Guid writerGuid{sourceGuidPrefix, submsgData->writerId};
        ReaderCacheChange change{ChangeKind_t::ALIVE, writerGuid, submsgData->writerSN, serializedData, size};
        reader->newChange(change);
    }else{
#if RECV_VERBOSE
        printf("Couldn't find a reader with id: ");
        printEntityId(submsgData->readerId);
        printf("\n");
#endif
    }

    return true;
}

bool MessageReceiver::processHeartbeatSubmessage(MessageProcessingInfo& msgInfo){
    auto submsgHB = reinterpret_cast<const SubmessageHeartbeat*>(msgInfo.getPointerToPos());

    Reader* reader = mp_part->getReader(submsgHB->readerId);
    if(reader != nullptr){
        reader->onNewHeartbeat(*submsgHB, sourceGuidPrefix);
        return true;
    }else{
        return false;
    }
}

bool MessageReceiver::processAckNackSubmessage(MessageProcessingInfo& msgInfo){
    auto submsgAckNack = reinterpret_cast<const SubmessageAckNack*>(msgInfo.getPointerToPos());
    Writer* writer = mp_part->getWriter(submsgAckNack->writerId);
    if(writer != nullptr){
        writer->onNewAckNack(*submsgAckNack);
        return true;
    }else{
        return false;
    }
}

#undef RECV_VERBOSE


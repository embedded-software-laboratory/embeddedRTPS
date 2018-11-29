/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageReceiver.h"

#include "rtps/entities/Reader.h"
#include "rtps/messages/MessageTypes.h"

using rtps::MessageReceiver;

MessageReceiver::MessageReceiver(GuidPrefix_t partGuid)
: ourGuid(partGuid){

}

void MessageReceiver::reset(){
    sourceGuidPrefix = GUIDPREFIX_UNKNOWN;
    sourceVersion = PROTOCOLVERSION;
    sourceVendor = VENDOR_UNKNOWN;
    haveTimeStamp = false;
}


bool MessageReceiver::addReader(Reader& reader){
    if(m_numReaders != m_readers.size()){
        m_readers[m_numReaders++] = &reader;
        return true;
    }else{
        return false;
    }
}

bool MessageReceiver::processMessage(const uint8_t* data, data_size_t size){

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

    if(header->guidPrefix.id == ourGuid.id){
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
        case SubmessageKind::DATA:
            printf("Processing DataSubMessage\n");
            success = processDataSubMessage(msgInfo);
            break;
        default:
            printf("Submessage of type %u currently not supported. Skipping..\n", static_cast<uint8_t>(submsgHeader->submessageId));
            success = false;
    }
    msgInfo.nextPos+= submsgHeader->submessageLength + sizeof(SubmessageHeader);
    return success;
}

bool MessageReceiver::processDataSubMessage(MessageProcessingInfo& msgInfo){
    auto submsgData = reinterpret_cast<const SubmessageData*>(msgInfo.getPointerToPos());
    const auto serializedData = msgInfo.getPointerToPos() + sizeof(SubmessageData);
    const data_size_t size = msgInfo.size - (msgInfo.nextPos - sizeof(SubmessageData));

    //if(submsgHeader->submessageLength > msgInfo.size - msgInfo.nextPos){
    //    return false;
    //}
    // TODO We can do better than that
    //bool isLittleEndian = (submsgHeader->flags & SubMessageFlag::FLAG_ENDIANESS);
    //bool hasInlineQos = (submsgHeader->flags & SubMessageFlag::FLAG_INLINE_QOS);


    for(uint16_t i=0; i<m_numReaders; ++i){
        static_assert(sizeof(i) > sizeof(m_numReaders), "Size of loop variable not sufficient");
        Reader& currentReader = *m_readers[i];
        if(currentReader.entityId == submsgData->readerId){
            currentReader.newChange(ChangeKind_t::ALIVE, serializedData, size);
            break;
        }
    }
    return true;
}


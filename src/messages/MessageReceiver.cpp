/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/messages/MessageReceiver.h"

#include "rtps/entities/Reader.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Writer.h"

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


bool MessageReceiver::addReader(Reader* pReader){
    if(pReader == nullptr){
        return false;
    }
    if(m_numReaders != m_readers.size()){
        m_readers[m_numReaders++] = pReader;
        return true;
    }else{
        return false;
    }
}

bool MessageReceiver::addWriter(Writer* pWriter){
    if(pWriter == nullptr){
        return false;
    }
    if(m_numWriters != m_writers.size()){
        m_writers[m_numWriters++] = pWriter;
        return true;
    }else{
        return false;
    }
}

void MessageReceiver::addBuiltInEndpoints(BuiltInEndpoints& endpoints){
    addWriter(endpoints.spdpWriter);
    addReader(endpoints.spdpReader);
    addWriter(endpoints.sedpPubWriter);
    addReader(endpoints.sedpPubReader);
    addWriter(endpoints.sedpSubWriter);
    addReader(endpoints.sedpSubReader);
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
            printf("Processing Data submessage\n");
            success = processDataSubmessage(msgInfo);
            break;
        case SubmessageKind::HEARTBEAT:
            printf("Processing Heartbeat submessage\n");
            success = processHeartbeatSubmessage(msgInfo);
            break;
        default:
            printf("Submessage of type %u currently not supported. Skipping..\n", static_cast<uint8_t>(submsgHeader->submessageId));
            success = false;
    }
    msgInfo.nextPos+= submsgHeader->submessageLength + sizeof(SubmessageHeader);
    return success;
}

bool MessageReceiver::processDataSubmessage(MessageProcessingInfo &msgInfo){
    auto submsgData = reinterpret_cast<const SubmessageData*>(msgInfo.getPointerToPos());
    const uint8_t* serializedData = msgInfo.getPointerToPos() + sizeof(SubmessageData);
    const DataSize_t size = msgInfo.size - (msgInfo.nextPos + sizeof(SubmessageData));

    //if(submsgHeader->submessageLength > msgInfo.size - msgInfo.nextPos){
    //    return false;
    //}
    // TODO We can do better than that
    //bool isLittleEndian = (submsgHeader->flags & SubMessageFlag::FLAG_ENDIANESS);
    //bool hasInlineQos = (submsgHeader->flags & SubMessageFlag::FLAG_INLINE_QOS);

    for(uint16_t i=0; i<m_numReaders; ++i){
        static_assert(sizeof(i) > sizeof(m_numReaders), "Size of loop variable not sufficient");
        Reader& currentReader = *m_readers[i];
        if(currentReader.m_guid.entityId == submsgData->readerId){
            Guid writerGuid{sourceGuidPrefix, submsgData->writerId};
            ReaderCacheChange change{ChangeKind_t::ALIVE, writerGuid, submsgData->writerSN, serializedData, size};
            currentReader.newChange(change);
            break;
        }
    }
    return true;
}

bool MessageReceiver::processHeartbeatSubmessage(MessageProcessingInfo &msgInfo){
    auto submsgHB = reinterpret_cast<const SubmessageHeartbeat*>(msgInfo.getPointerToPos());

    for(uint8_t i=0; i < m_numReaders; ++i){
        Reader& reader = *m_readers[i];
        if(reader.m_guid.entityId == submsgHB->readerId){
            reader.onNewHeartbeat(*submsgHB, sourceGuidPrefix);
            return true;
        }
    }

    return false;
}


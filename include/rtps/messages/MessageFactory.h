/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGEFACTORY_H
#define RTPS_MESSAGEFACTORY_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/rtps.h"

#include <cstdint>
#include <array>


namespace rtps{
    namespace MessageFactory{
        const std::array<uint8_t, 4> PROTOCOL_TYPE{'R', 'T', 'P', 'S'};
        const uint8_t numBytesUntilEndOfLength = 4; // The first bytes incl. submessagelength don't count

        template <class Buffer>
        void addHeader(Buffer& buffer, const GuidPrefix_t& guidPrefix){

            Header header;
            header.protocolName = PROTOCOL_TYPE;
            header.protocolVersion = PROTOCOLVERSION;
            header.vendorId = Config::VENDOR_ID;
            header.guidPrefix = guidPrefix;

            serializeMessage(buffer, header);
        }

        template <class Buffer>
        void addSubMessageTimeStamp(Buffer& buffer, bool setInvalid=false){
            SubmessageHeader header;
            header.submessageId = SubmessageKind::INFO_TS;

#if IS_LITTLE_ENDIAN
            header.flags = FLAG_LITTLE_ENDIAN;
#else
            header.flags = FLAG_BIG_ENDIAN;
#endif

            if(setInvalid){
                header.flags |= FLAG_INVALIDATE;
                header.submessageLength = 0;
            }else{
                header.submessageLength = sizeof(Time_t);
            }

            serializeMessage(buffer, header);

            if(!setInvalid){
                buffer.reserve(header.submessageLength);
                Time_t now = getCurrentTimeStamp();
                buffer.append(reinterpret_cast<uint8_t*>(&now.seconds), sizeof(Time_t::seconds));
                buffer.append(reinterpret_cast<uint8_t*>(&now.fraction), sizeof(Time_t::fraction));
            }

        }

        template <class Buffer>
        void addSubMessageData(Buffer& buffer, const Buffer& filledPayload, bool containsInlineQos, const SequenceNumber_t& SN, const EntityId_t& writerID, const EntityId_t& readerID){
            SubmessageData msg;
            msg.header.submessageId = SubmessageKind::DATA;
#if IS_LITTLE_ENDIAN
            msg.header.flags = FLAG_LITTLE_ENDIAN;
#else
            msg.header.flags = FLAG_BIG_ENDIAN;
#endif

            msg.header.submessageLength = SubmessageData::getRawSize() + filledPayload.getUsedSize() - numBytesUntilEndOfLength;

            if(containsInlineQos){
                msg.header.flags |= FLAG_INLINE_QOS;
            }
            if(filledPayload.isValid()){
                msg.header.flags |= FLAG_DATA_PAYLOAD;
            }

            msg.writerSN = SN;
            msg.extraFlags = 0;
            msg.readerId = readerID;
            msg.writerId = writerID;

            constexpr uint16_t octetsToInlineQoS = 4 + 4 + 8; // EntityIds + SequenceNumber
            msg.octetsToInlineQos = octetsToInlineQoS;

            serializeMessage(buffer, msg);

            if(filledPayload.isValid()){
                Buffer shallowCopy = filledPayload;
                buffer.append(std::move(shallowCopy));
            }
        }

        template <class Buffer>
        void addHeartbeat(Buffer& buffer, EntityId_t writerId, EntityId_t readerId, SequenceNumber_t firstSN,
                          SequenceNumber_t lastSN, Count_t count){
            SubmessageHeartbeat subMsg;
            subMsg.header.submessageId = SubmessageKind::HEARTBEAT;
            subMsg.header.submessageLength = SubmessageHeartbeat::getRawSize() - numBytesUntilEndOfLength;
#if IS_LITTLE_ENDIAN
            subMsg.header.flags = FLAG_LITTLE_ENDIAN;
#else
            subMsg.header.flags = FLAG_BIG_ENDIAN;
#endif
            // Force response by not setting final flag.

            subMsg.writerId = writerId;
            subMsg.readerId = readerId;
            subMsg.firstSN = firstSN;
            subMsg.lastSN = lastSN;
            subMsg.count = count;

            serializeMessage(buffer, subMsg);
        }

        template <class Buffer>
        void addAckNack(Buffer& buffer, EntityId_t writerId, EntityId_t readerId, SequenceNumberSet readerSNState,
                        Count_t count){
            SubmessageAckNack subMsg;
            subMsg.header.submessageId = SubmessageKind::ACKNACK;
#if IS_LITTLE_ENDIAN
            subMsg.header.flags = FLAG_LITTLE_ENDIAN;
#else
            subMsg.header.flags = FLAG_BIG_ENDIAN;
#endif
            subMsg.header.flags |= FLAG_FINAL; // For now, we don't want any response
            subMsg.header.submessageLength = SubmessageAckNack::getRawSize(readerSNState) - numBytesUntilEndOfLength;

            subMsg.writerId = writerId;
            subMsg.readerId = readerId;
            subMsg.readerSNState = readerSNState;
            subMsg.count = count;

            serializeMessage(buffer, subMsg);
        }
    }
}

#endif //RTPS_MESSAGEFACTORY_H

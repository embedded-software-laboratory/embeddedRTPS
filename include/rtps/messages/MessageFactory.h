/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGEFACTORY_H
#define RTPS_MESSAGEFACTORY_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/messages/Messages.h"
#include "rtps/rtps.h"

#include <cstdint>
#include <array>


namespace rtps{
    namespace MessageFactory{
        constexpr std::array<uint8_t, 4> PROTOCOL_TYPE{'R', 'T', 'P', 'S'};

        template <class Buffer>
        void addHeader(Buffer& buffer, const GuidPrefix_t& guidPrefix){

            Header header;
            header.protocolName = PROTOCOL_TYPE;
            header.protocolVersion = PROTOCOLVERSION;
            header.vendorId = Config::VENDOR_ID;
            header.guidPrefix = guidPrefix;

            header.serializeInto(buffer);
        };

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

            header.serializeInto(buffer);

            if(!setInvalid){
                buffer.reserve(header.submessageLength);
                Time_t now = getCurrentTimeStamp();
                buffer.append(reinterpret_cast<uint8_t*>(&now.seconds), sizeof(Time_t::seconds));
                buffer.append(reinterpret_cast<uint8_t*>(&now.fraction), sizeof(Time_t::fraction));
            }

        }

        template <class Buffer>
        void addSubMessageData(Buffer& buffer, const Buffer& filledPayload, bool containsInlineQos, const SequenceNumber_t& SN, const EntityId_t& writerID){
            SubmessageData msg;
            msg.header.submessageId = SubmessageKind::DATA;
#if IS_LITTLE_ENDIAN
            msg.header.flags = FLAG_LITTLE_ENDIAN;
#else
            msg.header.flags = FLAG_BIG_ENDIAN;
#endif
            const auto offset = 4; // I guess the encapsulation info (CDR scheme and options) doesn't count
            msg.header.submessageLength = sizeof(SubmessageData) + filledPayload.getSize() - offset;

            if(containsInlineQos){
                msg.header.flags |= FLAG_INLINE_QOS;
            }
            if(filledPayload.isValid()){
                msg.header.flags |= FLAG_DATA_PAYLOAD;
            }

            msg.writerSN = SN;
            msg.extraFlags = 0;
            msg.readerId = ENTITYID_UNKNOWN;
            msg.writerId = writerID;

            constexpr uint16_t octetsToInlineQoS = 4 + 4 + 8; // EntityIds + SequenceNumber
            msg.octetsToInlineQos = octetsToInlineQoS;

            buffer.reserve(sizeof(SubmessageData));
            msg.serializeInto(buffer);

            if(filledPayload.isValid()){
                PBufWrapper shallowCopy = filledPayload;
                buffer.append(std::move(shallowCopy));
            }
        }
    }
}

#endif //RTPS_MESSAGEFACTORY_H

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
        constexpr std::array<uint8_t, 2> PROTOCOL_VERSION{PROTOCOLVERSION.major, PROTOCOLVERSION.minor};

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
        void addSubMessageTimeStamp(Buffer& buffer, bool addValidTimestamp){
            SubmessageHeader header;
            header.submessageId = SubmessageKind::INFO_TS;

#if IS_LITTLE_ENDIAN
            header.flags = FLAG_LITTLE_ENDIAN;
#else
            header.flags = FLAG_BIG_ENDIAN;
#endif

            if(addValidTimestamp){
                header.submessageLength = sizeof(Time_t);
            }else{
                header.flags |= FLAG_INVALIDATE;
                header.submessageLength = 0;
            }

            header.serializeInto(buffer);

            if(addValidTimestamp){
                // TODO Add some timestamp.
                buffer.reserve(header.submessageLength);
                Time_t now = getCurrentTimeStamp();
                buffer.append(&now.seconds, sizeof(Time_t::seconds));
                buffer.append(&now.fraction, sizeof(Time_t::fraction));
            }

        }

        template <class Buffer>
        void addSubMessageData(Buffer& buffer, Buffer&& filledPayload, Buffer&& filledInlineQos){
            SubmessageData msg;
            msg.header.submessageId = SubmessageKind::DATA;
#if IS_LITTLE_ENDIAN
            msg.header.flags = FLAG_LITTLE_ENDIAN;
#else
            msg.header.flags = FLAG_BIG_ENDIAN;
#endif
            const auto sizeMessage = sizeof(SubmessageData) + filledPayload.getSize() + filledInlineQos.getSize();
            msg.header.submessageLength = sizeMessage - sizeof(SubmessageData::header);

            if(filledInlineQos.isValid()){
                msg.header.flags |= FLAG_INLINE_QOS;
            }
            if(filledPayload.isValid()){
                msg.header.flags |= FLAG_DATA_PAYLOAD;
            }

            buffer.reserve(sizeMessage);
            msg.serializeInto(buffer);

            if(filledInlineQos.isValid()){
                buffer.append(std::forward(filledInlineQos));
            }
            if(filledPayload.isValid()){
                buffer.append(std::forward(filledInlineQos));
            }
        }

        template <class Buffer>
        void addSPDPBCastMessage(Buffer& buffer, Buffer&& filledPayload, Buffer&& filledInlineQos){
            addSubMessageTimeStamp(buffer, true);

            addSubMessageData(buffer, std::forward(filledPayload), std::forward(filledInlineQos));
        }

    }
}

#endif //RTPS_MESSAGEFACTORY_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGES_H
#define RTPS_MESSAGES_H

#include "rtps/types.h"

#include <array>

namespace rtps{

    namespace SMElement{
        enum class ParameterId_t : uint8_t{
            PID_PAD                                 = 0x0000,
            PID_SENTINEL                            = 0x0001,
            PID_USER_DATA                           = 0x002c,
            PID_TOPIC_NAME                          = 0x0005,
            PID_TYPE_NAME                           = 0x0007,
            PID_GROUP_DATA                          = 0x002d,
            PID_TOPIC_DATA                          = 0x002e,
            PID_DURABILITY                          = 0x001d,
            PID_DURABILITY_SERVICE                  = 0x001e,
            PID_DEADLINE                            = 0x0023,
            PID_LATENCY_BUDGET                      = 0x0027,
            PID_LIVELINESS                          = 0x001b,
            PID_RELIABILITY                         = 0x001a,
            PID_LIFESPAN                            = 0x002b,
            PID_DESTINATION_ORDER                   = 0x0025,
            PID_HISTORY                             = 0x0040,
            PID_RESOURCE_LIMITS                     = 0x0041,
            PID_OWNERSHIP                           = 0x001f,
            PID_OWNERSHIP_STRENGTH                  = 0x0006,
            PID_PRESENTATION                        = 0x0021,
            PID_PARTITION                           = 0x0029,
            PID_TIME_BASED_FILTER                   = 0x0004,
            PID_TRANSPORT_PRIORITY                  = 0x0049,
            PID_PROTOCOL_VERSION                    = 0x0015,
            PID_VENDORID                            = 0x0016,
            PID_UNICAST_LOCATOR                     = 0x002f,
            PID_MULTICAST_LOCATOR                   = 0x0030,
            PID_MULTICAST_IPADDRESS                 = 0x0011,
            PID_DEFAULT_UNICAST_LOCATOR             = 0x0031,
            PID_DEFAULT_MULTICAST_LOCATOR           = 0x0048,
            PID_METATRAFFIC_UNICAST_LOCATOR         = 0x0032,
            PID_METATRAFFIC_MULTICAST_LOCATOR       = 0x0033,
            PID_DEFAULT_UNICAST_IPADDRESS           = 0x000c,
            PID_DEFAULT_UNICAST_PORT                = 0x000e,
            PID_METATRAFFIC_UNICAST_IPADDRESS       = 0x0045,
            PID_METATRAFFIC_UNICAST_PORT            = 0x000d,
            PID_METATRAFFIC_MULTICAST_IPADDRESS     = 0x000b,
            PID_METATRAFFIC_MULTICAST_PORT          = 0x0046,
            PID_EXPECTS_INLINE_QOS                  = 0x0043,
            PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT = 0x0034,
            PID_PARTICIPANT_BUILTIN_ENDPOINTS       = 0x0044,
            PID_PARTICIPANT_LEASE_DURATION          = 0x0002,
            PID_CONTENT_FILTER_PROPERTY             = 0x0035,
            PID_PARTICIPANT_GUID                    = 0x0050,
            PID_PARTICIPANT_ENTITYID                = 0x0051,
            PID_GROUP_GUID                          = 0x0052,
            PID_GROUP_ENTITYID                      = 0x0053,
            PID_BUILTIN_ENDPOINT_SET                = 0x0058,
            PID_PROPERTY_LIST                       = 0x0059,
            PID_TYPE_MAX_SIZE_SERIALIZED            = 0x0060,
            PID_ENTITY_NAME                         = 0x0062,
            PID_KEY_HASH                            = 0x0070,
            PID_STATUS_INFO                         = 0x0071
        };

        struct ParameterList{
            ParameterId_t parameterId;
            uint16_t length;
            uint8_t data[];
        };
    }
    // TODO
    constexpr uint8_t CDR_LE[] = {0,1};
    constexpr uint8_t CDR_BE[] = {0,0};

    enum class SubmessageKind : uint8_t{
        PAD             = 0x01, /* Pad */
        ACKNACK         = 0x06, /* AckNack */
        HEARTBEAT       = 0x07, /* Heartbeat */
        GAP             = 0x08, /* Gap */
        INFO_TS         = 0x09, /* InfoTimestamp */
        INFO_SRC        = 0x0c, /* InfoSource */
        INFO_REPLY_IP4  = 0x0d, /* InfoReplyIp4 */
        INFO_DST        = 0x0e, /* InfoDestination */
        INFO_REPLY      = 0x0f, /* InfoReply */
        NACK_FRAG       = 0x12, /* NackFrag */
        HEARTBEAT_FRAG  = 0x13, /* HeartbeatFrag */
        DATA            = 0x15, /* Data */
        DATA_FRAG       = 0x16 /* DataFrag */
    };

    enum SubMessageFlag{
        FLAG_BIG_ENDIAN     = (0 << 0),
        FLAG_LITTLE_ENDIAN  = (1 << 0),
        FLAG_INVALIDATE     = (1 << 1),
        FLAG_INLINE_QOS     = (1 << 1),
        FLAG_NO_PAYLOAD     = (0 << 3 | 0 << 2),
        FLAG_DATA_PAYLOAD   = (0 << 3 | 1 << 2),

    };

    struct Header{
        std::array<uint8_t, 4> protocolName;
        ProtocolVersion_t protocolVersion;
        VendorId_t vendorId;
        GuidPrefix_t guidPrefix;

        template <class Buffer>
        void serializeInto(Buffer &buffer){
            constexpr auto size = sizeof(Header);
            buffer.reserve(size);
            buffer.append(reinterpret_cast<uint8_t*>(this), size);
        }
    } __attribute__((packed));

    struct SubmessageHeader{
        SubmessageKind submessageId;
        uint8_t flags;
        uint16_t submessageLength;

        template <class Buffer>
        void serializeInto(Buffer &buffer){
            constexpr auto size = sizeof(SubmessageHeader);
            buffer.reserve(size);
            buffer.append(reinterpret_cast<uint8_t*>(this), size);
        }
    };

    struct SubmessageData{
        SubmessageHeader header;
        uint16_t extraFlags;
        uint16_t octetsToInlineQos;
        EntityId_t readerId;
        EntityId_t writerId;
        SequenceNumber_t writerSN;

        template <class Buffer>
        void serializeInto(Buffer &buffer){
            constexpr auto size = sizeof(SubmessageData);
            buffer.reserve(size);
            buffer.append(reinterpret_cast<uint8_t*>(this), size);

        }
    };
}

#endif //RTPS_MESSAGES_H

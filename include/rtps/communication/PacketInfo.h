/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PACKETINFO_H
#define RTPS_PACKETINFO_H

#include "rtps/common/types.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{

    struct PacketInfo{
        Ip4Port_t srcPort;
        ip4_addr_t destAddr;
        Ip4Port_t destPort;
        PBufWrapper buffer;

        void copyTriviallyCopyable(const PacketInfo& other){
            this->srcPort = other.srcPort; // TODO Do we need that?
            this->destPort = other.destPort;
            this->destAddr = other.destAddr;
        }

        PacketInfo() = default;
        ~PacketInfo() = default;

        PacketInfo& operator=(const PacketInfo& other){
            copyTriviallyCopyable(other);
            this->buffer = other.buffer;
            return *this;
        }

        PacketInfo& operator=(PacketInfo&& other) noexcept{
            copyTriviallyCopyable(other);
            this->buffer = std::move(other.buffer);
            return *this;
        }
    };
}

#endif //RTPS_PACKETINFO_H

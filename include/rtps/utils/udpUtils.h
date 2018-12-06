/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDP_UTILS_H
#define RTPS_UDP_UTILS_H


#include "rtps/config.h"
#include "rtps/common/Locator.h"

namespace rtps {
    namespace {
        const uint16_t PB = 7400; // Port Base Number
        const uint16_t DG = 250; // DomainId Gain
        const uint16_t PG = 2; // ParticipantId Gain
        // Additional Offsets
        const uint16_t D0 = 0; // Builtin multicast
        const uint16_t D1 = 10; // Builtin unicast
        const uint16_t D2 = 1; // User multicast
        const uint16_t D3 = 11; // User unicast
    }


    constexpr ip4_addr transformIP4ToU32(uint8_t MSB, uint8_t p2, uint8_t p1, uint8_t LSB) {
        return {((uint32_t) (LSB << 24)) |
                ((uint32_t) (p1 << 16)) |
                ((uint32_t) (p2 << 8)) |
                MSB};
    }

    inline Ip4Port_t getBuiltInUnicastPort(ParticipantId_t participantId) {
        return PB +
               DG * Config::DOMAIN_ID +
               D1 +
               PG * participantId;
    }

    constexpr Ip4Port_t getBuiltInMulticastPort() {
        return PB +
               DG * Config::DOMAIN_ID +
               D0;
    }

    inline Ip4Port_t getUserUnicastPort(ParticipantId_t participantId) {
        return PB +
               DG * Config::DOMAIN_ID +
               D3 +
               PG * participantId;
    }

    constexpr Ip4Port_t getUserMulticastPort() {
        return PB +
               DG * Config::DOMAIN_ID +
               D2;
    }

    inline bool isUserPort(Ip4Port_t port){
        return (port & 1) == 1;
    }

    inline bool isMultiCastPort(Ip4Port_t port){
        const auto idWithoutBase = port - PB - DG*Config::DOMAIN_ID;
        return idWithoutBase == D0 || idWithoutBase == D2;
    }

    inline ParticipantId_t getParticipantIdFromUnicastPort(Ip4Port_t port, bool isUserPort){
        const auto basePart = PB + DG*Config::DOMAIN_ID;
        ParticipantId_t participantPart = port - basePart;
        if(isUserPort){
            participantPart -= D3;
        }else{
            participantPart -= D1;
        }

        auto id = static_cast<ParticipantId_t>(participantPart / PG);
        if(id*PG + basePart == port){
            return id;
        }else{
            return PARTICIPANT_ID_INVALID;
        }
    }

    inline Locator getBuiltInUnicastLocator(ParticipantId_t participantId) {
        return Locator::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getBuiltInUnicastPort(participantId));
    }

    inline Locator getBuiltInMulticastLocator() {
        return Locator::createUDPv4Locator(239, 255, 0, 1, getBuiltInMulticastPort());
    }

    inline Locator getUserUnicastLocator(ParticipantId_t participantId) {
        return Locator::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getUserUnicastPort(participantId));
    }

    inline Locator getUserMulticastLocator() {
        return Locator::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getUserMulticastPort());
    }

    inline Locator getDefaultSendMulticastLocator() {
        return Locator::createUDPv4Locator(239, 255, 0, 1,
                                             getBuiltInMulticastPort());
    }
}

#endif //RTPS_UDP_H

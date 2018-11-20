/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_UDP_UTILS_H
#define RTPS_UDP_UTILS_H


#include "rtps/config.h"

namespace rtps {
    namespace {
        const uint16_t PB = 7400; // Port Base Number
        const uint16_t DG = 250; // DomainId Gain
        const uint16_t PG = 2; // ParticipantId Gain
        // Additional Offsets
        const uint16_t D0 = 0;
        const uint16_t D1 = 10;
        const uint16_t D2 = 1;
        const uint16_t D3 = 11;
    }


    constexpr ip4_addr transforIP4ToU32(uint8_t MSB, uint8_t p2, uint8_t p1, uint8_t LSB) {
        return {((uint32_t) (MSB << 24)) |
                ((uint32_t) (p2 << 16)) |
                ((uint32_t) (p1 << 8)) |
                LSB};
    }

    inline uint16_t getBuiltInUnicastPort(uint8_t participantId) {
        return PB +
               DG * Config::DOMAIN_ID +
               D1 +
               PG * participantId;
    }

    constexpr uint16_t getBuiltInMulticastPort() {
        return PB +
               DG * Config::DOMAIN_ID +
               D0;
    }

    inline uint16_t getUserUnicastPort(uint8_t participantId) {
        return PB +
               DG * Config::DOMAIN_ID +
               D3 +
               PG * participantId;
    }

    constexpr uint16_t getUserMulticastPort() {
        return PB +
               DG * Config::DOMAIN_ID +
               D2;
    }

    inline Locator_t getBuiltInUnicastLocator(participantId_t participantId) {
        return Locator_t::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getBuiltInUnicastPort(participantId));
    }

    inline Locator_t getBuiltInMulticastLocator() {
        return Locator_t::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getBuiltInMulticastPort());
    }

    inline Locator_t getUserUnicastLocator(participantId_t participantId) {
        return Locator_t::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getUserUnicastPort(participantId));
    }

    inline Locator_t getUserMulticastLocator() {
        return Locator_t::createUDPv4Locator(Config::IP_ADDRESS[0], Config::IP_ADDRESS[1],
                                             Config::IP_ADDRESS[2], Config::IP_ADDRESS[3],
                                             getUserMulticastPort());
    }

    inline Locator_t getDefaultSendMulticastLocator() {
        return Locator_t::createUDPv4Locator(239, 255, 0, 1,
                                             getBuiltInMulticastPort());
    }
}

#endif //RTPS_UDP_H

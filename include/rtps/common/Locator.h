/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#ifndef RTPS_LOCATOR_T_H
#define RTPS_LOCATOR_T_H

#include "rtps/communication/UdpDriver.h"
#include "rtps/utils/udpUtils.h"
#include "ucdr/microcdr.h"

#include <array>

namespace rtps {
enum class LocatorKind_t : int32_t {
  LOCATOR_KIND_INVALID = -1,
  LOCATOR_KIND_RESERVED = 0,
  LOCATOR_KIND_UDPv4 = 1,
  LOCATOR_KIND_UDPv6 = 2
};

const uint32_t LOCATOR_PORT_INVALID = 0;
const std::array<uint8_t, 16> LOCATOR_ADDRESS_INVALID = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*
 * This representation corresponds to the RTPS wire format
 */
struct FullLengthLocator {
  LocatorKind_t kind = LocatorKind_t::LOCATOR_KIND_INVALID;
  uint32_t port = LOCATOR_PORT_INVALID;
  std::array<uint8_t, 16> address =
      LOCATOR_ADDRESS_INVALID; // TODO make private such that kind and address
                               // always match?

  static FullLengthLocator createUDPv4Locator(uint8_t a, uint8_t b, uint8_t c,
                                              uint8_t d, uint32_t port) {
    FullLengthLocator locator;
    locator.kind = LocatorKind_t::LOCATOR_KIND_UDPv4;
    locator.address = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, a, b, c, d};
    locator.port = port;
    return locator;
  }

  void setInvalid() { kind = LocatorKind_t::LOCATOR_KIND_INVALID; }

  bool isValid() const { return kind != LocatorKind_t::LOCATOR_KIND_INVALID; }

  bool readFromUcdrBuffer(ucdrBuffer &buffer) {
    if (ucdr_buffer_remaining(&buffer) < sizeof(FullLengthLocator)) {
      return false;
    } else {
      ucdr_deserialize_array_uint8_t(&buffer, reinterpret_cast<uint8_t *>(this),
                                     sizeof(FullLengthLocator));
      return true;
    }
  }

  bool serializeIntoUdcrBuffer(ucdrBuffer &buffer) {
    if (ucdr_buffer_remaining(&buffer) < sizeof(FullLengthLocator)) {
      return false;
    } else {
      ucdr_serialize_array_uint8_t(&buffer, reinterpret_cast<uint8_t *>(this),
                                   sizeof(FullLengthLocator));
    }
  }

  ip4_addr_t getIp4Address() const {
    return transformIP4ToU32(address[12], address[13], address[14],
                             address[15]);
  }

  bool isSameAddress(ip4_addr_t *address) {
    ip4_addr_t ownaddress = getIp4Address();
    return ip4_addr_cmp(&ownaddress, address);
  }

  inline bool isSameSubnet() const {
    return UdpDriver::isSameSubnet(getIp4Address());
  }

  inline bool isMulticastAddress() const {
    return UdpDriver::isMulticastAddress(getIp4Address());
  }

  inline uint32_t getLocatorPort() { return static_cast<Ip4Port_t>(port); }

} __attribute__((packed));

inline FullLengthLocator
getBuiltInUnicastLocator(ParticipantId_t participantId) {
  return FullLengthLocator::createUDPv4Locator(
      Config::IP_ADDRESS[0], Config::IP_ADDRESS[1], Config::IP_ADDRESS[2],
      Config::IP_ADDRESS[3], getBuiltInUnicastPort(participantId));
}

inline FullLengthLocator getBuiltInMulticastLocator() {
  return FullLengthLocator::createUDPv4Locator(239, 255, 0, 1,
                                               getBuiltInMulticastPort());
}

inline FullLengthLocator getUserUnicastLocator(ParticipantId_t participantId) {
  return FullLengthLocator::createUDPv4Locator(
      Config::IP_ADDRESS[0], Config::IP_ADDRESS[1], Config::IP_ADDRESS[2],
      Config::IP_ADDRESS[3], getUserUnicastPort(participantId));
}

inline FullLengthLocator
getUserMulticastLocator() { // this would be a unicastaddress, as
                            // defined in config
  return FullLengthLocator::createUDPv4Locator(
      Config::IP_ADDRESS[0], Config::IP_ADDRESS[1], Config::IP_ADDRESS[2],
      Config::IP_ADDRESS[3], getUserMulticastPort());
}

inline FullLengthLocator getDefaultSendMulticastLocator() {
  return FullLengthLocator::createUDPv4Locator(239, 255, 0, 1,
                                               getBuiltInMulticastPort());
}

/*
 * This representation omits unnecessary 12 bytes of the full RTPS wire format
 */
struct LocatorIPv4 {
  LocatorKind_t kind = LocatorKind_t::LOCATOR_KIND_INVALID;
  std::array<uint8_t, 4> address = {0};
  uint32_t port = LOCATOR_PORT_INVALID;

  LocatorIPv4() = default;
  LocatorIPv4(const FullLengthLocator &locator) {
    address[0] = locator.address[12];
    address[1] = locator.address[13];
    address[2] = locator.address[14];
    address[3] = locator.address[15];
    port = locator.port;
    kind = locator.kind;
  }

  ip4_addr_t getIp4Address() const {
    return transformIP4ToU32(address[0], address[1], address[2], address[3]);
  }

  void setInvalid() { kind = LocatorKind_t::LOCATOR_KIND_INVALID; }

  bool isValid() const { return kind != LocatorKind_t::LOCATOR_KIND_INVALID; }

  inline bool isSameSubnet() const {
    return UdpDriver::isSameSubnet(getIp4Address());
  }

  inline bool isMulticastAddress() const {
    return UdpDriver::isMulticastAddress(getIp4Address());
  }
};

} // namespace rtps

#endif // RTPS_LOCATOR_T_H

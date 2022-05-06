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

#include "rtps/discovery/SPDPAgent.h"
#include "rtps/entities/Participant.h"
#include "rtps/utils/Log.h"

using rtps::ParticipantProxyData;

ParticipantProxyData::ParticipantProxyData(Guid_t guid) : m_guid(guid) {}

void ParticipantProxyData::reset() {
  m_guid = Guid_t{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN};
  m_manualLivelinessCount = Count_t{1};
  m_expectsInlineQos = false;
  for (int i = 0; i < Config::SPDP_MAX_NUM_LOCATORS; ++i) {
    m_metatrafficUnicastLocatorList[i].setInvalid();
    m_metatrafficMulticastLocatorList[i].setInvalid();
    m_defaultUnicastLocatorList[i].setInvalid();
    m_defaultMulticastLocatorList[i].setInvalid();
  }
}

bool ParticipantProxyData::readFromUcdrBuffer(ucdrBuffer &buffer,
                                              Participant *participant) {
  reset();
  SMElement::ParameterId pid;
  uint16_t length;
  while (ucdr_buffer_remaining(&buffer) >= 4) {
    ucdr_deserialize_uint16_t(&buffer, reinterpret_cast<uint16_t *>(&pid));

    ucdr_deserialize_uint16_t(&buffer, &length);
    if (ucdr_buffer_remaining(&buffer) < length) {
      return false;
    }

    switch (pid) {
    case ParameterId::PID_KEY_HASH: {
      // TODO
      break;
    }

    case ParameterId::PID_PROTOCOL_VERSION: {
      ucdr_deserialize_uint8_t(&buffer, &m_protocolVersion.major);
      if (m_protocolVersion.major < PROTOCOLVERSION.major) {
        return false;
      } else {
        ucdr_deserialize_uint8_t(&buffer, &m_protocolVersion.minor);
      }
      break;
    }
    case ParameterId::PID_VENDORID: {
      ucdr_deserialize_array_uint8_t(&buffer, m_vendorId.vendorId.data(),
                                     m_vendorId.vendorId.size());
      break;
    }

    case ParameterId::PID_EXPECTS_INLINE_QOS: {
      ucdr_deserialize_bool(&buffer, &m_expectsInlineQos);
      break;
    }
    case ParameterId::PID_PARTICIPANT_GUID: {
      ucdr_deserialize_array_uint8_t(&buffer, m_guid.prefix.id.data(),
                                     m_guid.prefix.id.size());
      ucdr_deserialize_array_uint8_t(&buffer, m_guid.entityId.entityKey.data(),
                                     m_guid.entityId.entityKey.size());
      ucdr_deserialize_uint8_t(
          &buffer, reinterpret_cast<uint8_t *>(&m_guid.entityId.entityKind));
      if (participant->findRemoteParticipant(m_guid.prefix)) {
        SPDP_LOG("stopping deserialization early, participant is known\n");
        return true;
      }
      break;
    }
    case ParameterId::PID_METATRAFFIC_MULTICAST_LOCATOR: {
      if (!readLocatorIntoList(buffer, m_metatrafficMulticastLocatorList)) {
        return false;
      }
      break;
    }
    case ParameterId::PID_METATRAFFIC_UNICAST_LOCATOR: {
      if (!readLocatorIntoList(buffer, m_metatrafficUnicastLocatorList)) {
        return false;
      }
      break;
    }
    case ParameterId::PID_DEFAULT_UNICAST_LOCATOR: {
      if (!readLocatorIntoList(buffer, m_defaultUnicastLocatorList)) {
        return false;
      }
      break;
    }
    case ParameterId::PID_DEFAULT_MULTICAST_LOCATOR: {
      if (!readLocatorIntoList(buffer, m_defaultMulticastLocatorList)) {
        return false;
      }
      break;
    }
    case ParameterId::PID_PARTICIPANT_LEASE_DURATION: {
      ucdr_deserialize_int32_t(&buffer, &m_leaseDuration.seconds);
      ucdr_deserialize_uint32_t(&buffer, &m_leaseDuration.fraction);
      break;
    }
    case ParameterId::PID_BUILTIN_ENDPOINT_SET: {
      ucdr_deserialize_uint32_t(&buffer, &m_availableBuiltInEndpoints);
      break;
    }
    case ParameterId::PID_ENTITY_NAME: {
      // TODO
      buffer.iterator += length;
      buffer.last_data_size = 1;
      break;
    }
    case ParameterId::PID_PROPERTY_LIST: {
      // TODO
      buffer.iterator += length;
      buffer.last_data_size = 1;
      break;
    }
    case ParameterId::PID_USER_DATA: {
      // TODO
      buffer.iterator += length;
      buffer.last_data_size = 1;
      break;
    }
    case ParameterId::PID_PAD: {
      buffer.iterator += length;
      buffer.last_data_size = 1;
      break;
    }
    case ParameterId::PID_SENTINEL: {
      return true;
    }
    default: { return false; }
    }
    // Parameter lists are 4-byte aligned
    uint32_t alignment = ucdr_buffer_alignment(&buffer, 4);
    buffer.iterator += alignment;
    buffer.last_data_size = 4;
  }
  return true;
}

bool ParticipantProxyData::readLocatorIntoList(
    ucdrBuffer &buffer,
    std::array<LocatorIPv4, Config::SPDP_MAX_NUM_LOCATORS> &list) {
  int valid_locators = 0;
  FullLengthLocator full_length_locator;
  for (auto &proxy_locator : list) {
    if (!proxy_locator.isValid()) {
      bool ret = full_length_locator.readFromUcdrBuffer(buffer);
      if (ret && full_length_locator.isSameSubnet() ||
          full_length_locator.isMulticastAddress()) {
        proxy_locator = LocatorIPv4(full_length_locator);
        SPDP_LOG("Adding locator: %u %u %u %u \n",
                 (int)proxy_locator.address[0], (int)proxy_locator.address[1],
                 (int)proxy_locator.address[2],
                 (int)proxy_locator.address[3]);
        return true;
      } else {
        SPDP_LOG("Ignoring locator: %u %u %u %u \n",
                 (int)full_length_locator.address[12],
                 (int)full_length_locator.address[13],
                 (int)full_length_locator.address[14],
                 (int)full_length_locator.address[15]);
        return true;
      }
    } else {
      valid_locators++;
      if (valid_locators == Config::SPDP_MAX_NUM_LOCATORS) {
        buffer.iterator += sizeof(FullLengthLocator);
        SPDP_LOG("Max number of valid locators exceed, ignoring this locator "
                 "as we have at least one valid locator\n");
        return true;
      }
    }
  }
  return false;
}

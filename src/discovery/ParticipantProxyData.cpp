/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/ParticipantProxyData.h"

using rtps::ParticipantProxyData;

void ParticipantProxyData::reset(){
    m_guid = Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN};
    m_manualLivelinessCount = Count_t{1};
    m_expectsInlineQos = false;
    for(int i=0; i < 10; ++i){
        m_metatrafficUnicastLocatorList[i].setInvalid();
        m_metatrafficMulticastLocatorList[i].setInvalid();
        m_defaultUnicastLocatorList[i].setInvalid();
        m_defaultMulticastLocatorList[i].setInvalid();
    }
}

bool ParticipantProxyData::readFromUcdrBuffer(ucdrBuffer& buffer){
    reset();
    SMElement::ParameterId pid;
    uint16_t length;
    while(ucdr_buffer_remaining(&buffer) >= 4){
        ucdr_deserialize_uint16_t(&buffer, reinterpret_cast<uint16_t*>(&pid));

        ucdr_deserialize_uint16_t(&buffer, &length);
        if(ucdr_buffer_remaining(&buffer) < length){
            return false;
        }

        switch(pid){
            case ParameterId::PID_KEY_HASH:
            {
                // TODO
                break;
            }

            case ParameterId::PID_PROTOCOL_VERSION:
            {
                ucdr_deserialize_uint8_t(&buffer, &m_protocolVersion.major);
                if(m_protocolVersion.major < PROTOCOLVERSION.major){
                    return false;
                }else{
                    ucdr_deserialize_uint8_t(&buffer, &m_protocolVersion.minor);
                }
                break;
            }
            case ParameterId::PID_VENDORID:
            {
                ucdr_deserialize_array_uint8_t(&buffer, m_vendorId.vendorId.data(), m_vendorId.vendorId.size());
                break;
            }

            case ParameterId::PID_EXPECTS_INLINE_QOS:
            {
                ucdr_deserialize_bool(&buffer, &m_expectsInlineQos);
                break;
            }
            case ParameterId::PID_PARTICIPANT_GUID:
            {
                ucdr_deserialize_array_uint8_t(&buffer, m_guid.prefix.id.data(), m_guid.prefix.id.size());
                ucdr_deserialize_array_uint8_t(&buffer, m_guid.entityId.entityKey.data(), m_guid.entityId.entityKey.size());
                ucdr_deserialize_uint8_t(&buffer, reinterpret_cast<uint8_t*>(&m_guid.entityId.entityKind));
                break;
            }
            case ParameterId::PID_METATRAFFIC_MULTICAST_LOCATOR:
            {
                if(!readLocatorIntoList(buffer, m_metatrafficMulticastLocatorList)) {
                    return false;
                }
                break;
            }
            case ParameterId::PID_METATRAFFIC_UNICAST_LOCATOR:
            {
                if(!readLocatorIntoList(buffer, m_metatrafficUnicastLocatorList)) {
                    return false;
                }
                break;
            }
            case ParameterId::PID_DEFAULT_UNICAST_LOCATOR:
            {
                if(!readLocatorIntoList(buffer, m_defaultUnicastLocatorList)) {
                    return false;
                }
                break;
            }
            case ParameterId::PID_DEFAULT_MULTICAST_LOCATOR:
            {
                if(!readLocatorIntoList(buffer, m_defaultMulticastLocatorList)) {
                    return false;
                }
                break;
            }
            case ParameterId::PID_PARTICIPANT_LEASE_DURATION:
            {
                ucdr_deserialize_int32_t(&buffer, &m_leaseDuration.seconds);
                ucdr_deserialize_uint32_t(&buffer, &m_leaseDuration.fraction);
                break;
            }
            case ParameterId::PID_BUILTIN_ENDPOINT_SET:
            {
                ucdr_deserialize_uint32_t(&buffer, &m_availableBuiltInEndpoints);
                break;
            }
            case ParameterId::PID_ENTITY_NAME:
            {
                // TODO
                buffer.iterator+=length;
                buffer.last_data_size = 1;
                break;
            }
            case ParameterId::PID_PROPERTY_LIST:
            {
                // TODO
                buffer.iterator+=length;
                buffer.last_data_size = 1;
                break;
            }
            case ParameterId::PID_USER_DATA:
            {
                // TODO
                buffer.iterator+=length;
                buffer.last_data_size = 1;
                break;
            }
            case ParameterId::PID_PAD:
            {
                buffer.iterator+=length;
                buffer.last_data_size = 1;
                break;
            }
            case ParameterId::PID_SENTINEL:
            {
                return true;
            }
            default:
            {
                return false;
            }

        }
        // Parameter lists are 4-byte aligned
        uint32_t alignment = ucdr_buffer_alignment(&buffer, 4);
        buffer.iterator += alignment;
        buffer.last_data_size = 4;
    }
    return true;
}

bool ParticipantProxyData::readLocatorIntoList(ucdrBuffer& buffer, std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS>& list){
    for(auto& loc : list){
        if(!loc.isValid()){
            loc.readFromUcdrBuffer(buffer);
            return true;
        }
    }
    printf("SPDP: m_metatrafficMulticastLocatorList full.");
    return false;
}


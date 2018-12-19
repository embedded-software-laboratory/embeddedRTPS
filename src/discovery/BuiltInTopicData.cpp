/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include "rtps/discovery/BuiltInTopicData.h"
#include "rtps/messages/MessageTypes.h"

using rtps::BuiltInTopicData;
using rtps::SMElement::ParameterId;

bool BuiltInTopicData::readFromUcdrBuffer(ucdrBuffer& buffer){

    while(ucdr_buffer_remaining(&buffer) >= 4){
        ParameterId pid;
        uint16_t length;
        ucdr_deserialize_uint16_t(&buffer, reinterpret_cast<uint16_t*>(&pid));
        ucdr_deserialize_uint16_t(&buffer, &length);

        if(ucdr_buffer_remaining(&buffer) < length){
            return false;
        }

        switch(pid){
            case ParameterId::PID_ENDPOINT_GUID:
                ucdr_deserialize_array_uint8_t(&buffer, endpointGuid.prefix.id.data(), endpointGuid.prefix.id.size());
                ucdr_deserialize_array_uint8_t(&buffer, endpointGuid.entityId.entityKey.data(), endpointGuid.entityId.entityKey.size());
                ucdr_deserialize_uint8_t(&buffer, reinterpret_cast<uint8_t*>(&endpointGuid.entityId.entityKind));
                break;
            case ParameterId::PID_RELIABILITY:
                ucdr_deserialize_uint32_t(&buffer, reinterpret_cast<uint32_t*>(&reliabilityKind));
                buffer.iterator+=8;
                //TODO Skip 8 bytes. don't know what they are yet
                break;
            case ParameterId::PID_SENTINEL:
                return true;
            case ParameterId::PID_TOPIC_NAME:
                //TODO Skip 4 bytes. Don't know what they are yet.
                buffer.iterator+=4;
                length -=4;
                ucdr_deserialize_array_char(&buffer, topicName, length);
                break;
            case ParameterId ::PID_TYPE_NAME:
                //TODO Skip 4 bytes. Don't know what they are yet.
                buffer.iterator+=4;
                length -=4;
                ucdr_deserialize_array_char(&buffer, typeName, length);
                break;
            case ParameterId ::PID_UNICAST_LOCATOR:
                unicastLocator.readFromUcdrBuffer(buffer);
                break;
            default:
                buffer.iterator+=length;
                buffer.last_data_size = 1;
        }

        uint32_t alignment = ucdr_buffer_alignment(&buffer, 4);
        buffer.iterator += alignment;
        buffer.last_data_size = 4; // 4 Byte alignment per element
    }
    return ucdr_buffer_remaining(&buffer) == 0;
}

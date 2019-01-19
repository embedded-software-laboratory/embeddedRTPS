/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_LATENCYTOPIC_H
#define RTPS_LATENCYTOPIC_H

#include "rtps/config.h"

namespace rtps {
    namespace tests {

        class LatencyPacket{
        public:
            static constexpr const char* topicName = "LatencyTopic";
            static constexpr const char* typeName = "LatencyPacket";
            std::vector<uint8_t> data;

            static_assert(sizeof(topicName) < Config::MAX_TOPICNAME_LENGTH, "Topic name too long");
            static_assert(sizeof(typeName) < Config::MAX_TYPENAME_LENGTH, "Type name too long.");

            LatencyPacket(uint32_t size) : data(size){};
        };
    }
}

#endif //RTPS_LATENCYTOPIC_H

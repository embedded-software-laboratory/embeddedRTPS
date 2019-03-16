/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DISCOVEREDWRITERDATA_H
#define RTPS_DISCOVEREDWRITERDATA_H

#include <array>
#include "rtps/config.h"
#include "rtps/common/Locator.h"
#include "ucdr/microcdr.h"

namespace rtps{

    struct BuiltInTopicKey{
        std::array<uint32_t, 3> value;
    };

    struct TopicData{
        Guid endpointGuid;
        char typeName[Config::MAX_TYPENAME_LENGTH];
        char topicName[Config::MAX_TOPICNAME_LENGTH];
        ReliabilityKind_t reliabilityKind;
        Locator unicastLocator;

        TopicData() = default;
        TopicData(Guid guid, ReliabilityKind_t reliability, Locator loc)
            : endpointGuid(guid), typeName{'\0'}, topicName{'\0'},
              reliabilityKind(reliability), unicastLocator(loc){}


        bool matchesTopicOf(const TopicData& other);

        bool readFromUcdrBuffer(ucdrBuffer& buffer);
        bool serializeIntoUcdrBuffer(ucdrBuffer& buffer) const;
    };
}

#endif //RTPS_DISCOVEREDWRITERDATA_H






























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

    struct BuiltInTopicData{
        //BuiltInTopicKey key;
        //BuiltInTopicKey participantKey;
        ReliabilityKind_t reliabilityKind;
        char typeName[Config::MAX_TYPENAME_LENGTH];
        char topicName[Config::MAX_TOPICNAME_LENGTH];
        Locator unicastLocator;
        Guid endpointGuid;

        bool readFromUcdrBuffer(ucdrBuffer& buffer);
    };
}

#endif //RTPS_DISCOVEREDWRITERDATA_H






























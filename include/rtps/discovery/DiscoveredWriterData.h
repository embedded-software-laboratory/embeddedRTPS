/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DISCOVEREDWRITERDATA_H
#define RTPS_DISCOVEREDWRITERDATA_H

namespace rtps{
    struct DiscoveredWriterData{
        BuiltInTopicKey key;
        BuiltInTopicKey participantKey;
        char typeName[Config::MAX_TYPENAME_LENGTH];
        char topicName[Config::MAX_TOPICNAME_LENGTH];

    };
}

#endif //RTPS_DISCOVEREDWRITERDATA_H

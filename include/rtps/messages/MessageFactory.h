/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGEFACTORY_H
#define RTPS_MESSAGEFACTORY_H

#include "rtps/types.h"

#include <cstdint>
#include <array>

namespace rtps{
    namespace MessageFactory{
        constexpr std::array<uint8_t, 4> PROTOCOL_TYPE_DATA{'R', 'T', 'P', 'S'};
        constexpr std::array<uint8_t, 2> PROTOCOL_VERSION_DATA{PROTOCOLVERSION.major, PROTOCOLVERSION.minor};
        template <class Buffer>
        void addHeader(Buffer& buffer){
            constexpr data_size_t neededMemory = PROTOCOL_TYPE_DATA.size() +
                                                 PROTOCOL_VERSION_DATA.size();
            buffer.reserve(neededMemory);

            buffer.append(PROTOCOL_TYPE_DATA.data(), PROTOCOL_TYPE_DATA.size());
            buffer.append(PROTOCOL_VERSION_DATA.data(), PROTOCOL_VERSION_DATA.size());
        };

    }
}

#endif //RTPS_MESSAGEFACTORY_H

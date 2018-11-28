/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGERECEIVER_H
#define RTPS_MESSAGERECEIVER_H

#include <cstdint>
#include "rtps/types.h"
#include "rtps/config.h"

namespace rtps {
    class Reader;

    class MessageReceiver {
    public:
        GuidPrefix_t sourceGuidPrefix;
        ProtocolVersion_t sourceVersion;
        VendorId_t sourceVendor;
        bool haveTimeStamp = false;

        bool addWriter(); // for acknack etc.
        bool addReader(Reader& reader); // for new CacheChanges

        bool processMessage(const uint8_t* data, data_size_t size);

    private:
        struct MessageProcessingInfo{
            MessageProcessingInfo(const uint8_t* data, data_size_t size)
                : data(data), size(size){}
            const uint8_t* data;
            const data_size_t size;
            data_size_t nextPos = 0;

            inline const uint8_t* getPointerToPos(){
                return &data[nextPos];
            }

            inline data_size_t getRemainingSize(){
                return size - nextPos;
            }
        };

        std::array<Reader*, Config::NUM_READERS_PER_PARTICIPANT> m_readers{nullptr};
        uint8_t m_numReaders = 0;

        // TODO make msgInfo a member
        // This probably make processing faster, as no parameter needs to be passed around
        // However, we need to make sure data is set to nullptr after processMsg to make sure
        // we don't access it again afterwards.
        /**
         * Check header for validity, modifys the state of the receiver and
         * adjusts the position of msgInfo accordingly
         */
        bool processHeader(MessageProcessingInfo& msgInfo);
        bool processSubMessage(MessageProcessingInfo& msgInfo);
        bool processDataSubMessage(MessageProcessingInfo& msgInfo);
    };
}

#endif //RTPS_MESSAGERECEIVER_H

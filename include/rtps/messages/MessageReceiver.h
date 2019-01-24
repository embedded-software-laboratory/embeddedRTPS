/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MESSAGERECEIVER_H
#define RTPS_MESSAGERECEIVER_H

#include <cstdint>
#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/BuiltInEndpoints.h"

namespace rtps {
    class Reader;
    class Writer;
    class Participant;
    class MessageProcessingInfo;

    class MessageReceiver {
    public:
        GuidPrefix_t sourceGuidPrefix = GUIDPREFIX_UNKNOWN;
        ProtocolVersion_t sourceVersion = PROTOCOLVERSION;
        VendorId_t sourceVendor = VENDOR_UNKNOWN;
        bool haveTimeStamp = false;

        explicit MessageReceiver(Participant* part);

        void reset();

        bool processMessage(const uint8_t* data, DataSize_t size);

    private:

        Participant* mp_part;

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
        bool processDataSubmessage(MessageProcessingInfo& msgInfo);
        bool processHeartbeatSubmessage(MessageProcessingInfo& msgInfo);
        bool processAckNackSubmessage(MessageProcessingInfo& msgInfo);
    };
}

#endif //RTPS_MESSAGERECEIVER_H

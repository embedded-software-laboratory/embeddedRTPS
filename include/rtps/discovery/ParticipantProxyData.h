/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PARTICIPANTPROXYDATA_H
#define RTPS_PARTICIPANTPROXYDATA_H

#include "rtps/config.h"
#include "rtps/common/Locator.h"
#include "rtps/messages/MessageTypes.h"
#include "ucdr/microcdr.h"

#include <array>

namespace rtps{

    using SMElement::ParameterId;

    typedef uint32_t BuiltinEndpointSet_t;

    class ParticipantProxyData{
    public:

        ProtocolVersion_t m_protocolVersion = PROTOCOLVERSION;
        Guid m_guid = Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN};
        VendorId_t m_vendorId = VENDOR_UNKNOWN;
        bool m_expectsInlineQos = false;
        BuiltinEndpointSet_t m_availableBuiltInEndpoints;
        std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS> m_metatrafficUnicastLocatorList;
        std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS> m_metatrafficMulticastLocatorList;
        std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS> m_defaultUnicastLocatorList;
        std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS> m_defaultMulticastLocatorList;
        Count_t m_manualLivelinessCount{1};
        Duration_t m_leaseDuration = Config::SPDP_LEASE_DURATION;

        void reset();

        bool readFromUcdrBuffer(ucdrBuffer& buffer);

        inline bool hasParticipantWriter();
        inline bool hasParticipantReader();
        inline bool hasPublicationWriter();
        inline bool hasPublicationReader();
        inline bool hasSubscriptionWriter();
        inline bool hasSubscriptionReader();

    private:
        bool readLocatorIntoList(ucdrBuffer& buffer, std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS>& list);

        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER          = 1 << 0;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR           = 1 << 1;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER          = 1 << 2;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR           = 1 << 3;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER         = 1 << 4;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR          = 1 << 5;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER    = 1 << 6;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR     = 1 << 7;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER    = 1 << 8;
        static const BuiltinEndpointSet_t DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR     = 1 << 9;
        static const BuiltinEndpointSet_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER     = 1 << 10;
        static const BuiltinEndpointSet_t BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER     = 1 << 11;

    };

    // Needs to be in header because they are marked with inline
    bool ParticipantProxyData::hasParticipantWriter(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER) == 1;
    }

    bool ParticipantProxyData::hasParticipantReader(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR) != 0;
    }

    bool ParticipantProxyData::hasPublicationWriter(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER) != 0;
    }

    bool ParticipantProxyData::hasPublicationReader(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR) != 0;
    }

    bool ParticipantProxyData::hasSubscriptionWriter(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER) != 0;
    }

    bool ParticipantProxyData::hasSubscriptionReader(){
        return (m_availableBuiltInEndpoints & DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR) != 0;
    }
}
#endif //RTPS_PARTICIPANTPROXYDATA_H

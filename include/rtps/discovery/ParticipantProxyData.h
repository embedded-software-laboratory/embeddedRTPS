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
        Count_t m_manualLivelinessCount = Count_t{0};
        Duration_t m_leaseDuration = Config::SPDP_LEASE_DURATION;

        void reset();

        bool readFromUcdrBuffer(ucdrBuffer& buffer);

    private:
        bool readLocatorIntoList(ucdrBuffer& buffer, std::array<Locator, Config::SPDP_MAX_NUM_LOCATORS>& list);
    };

}
#endif //RTPS_PARTICIPANTPROXYDATA_H

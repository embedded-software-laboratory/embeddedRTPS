/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/common/types.h"
#include "rtps/common/Locator.h"
#include "rtps/discovery/ParticipantProxyData.h"

using rtps::ProtocolVersion_t;
using rtps::ParticipantProxyData;
using rtps::VendorId_t;
using rtps::Guid;
using rtps::Locator;
using rtps::Duration_t;
using rtps::BuiltinEndpointSet_t;

class ParticipantProxy : public ::testing::Test{
protected:
    // Extracted data from wireshark
    uint8_t parameterListFromEprosima[168] = {
            /*0060*/                                       0x15, 0x00, 0x04, 0x00, 0x02, 0x01, 0x00, 0x00, 0x16, 0x00,
            /*0070*/   0x04, 0x00, 0x01, 0x0f, 0x00, 0x00, 0x50, 0x00, 0x10, 0x00, 0x01, 0x0f, 0x00, 0x2d, 0x9f, 0x7e,
            /*0080*/   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc1, 0x33, 0x00, 0x18, 0x00, 0x01, 0x00,
            /*0090*/   0x00, 0x00, 0xe8, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /*00a0*/   0x00, 0x00, 0xef, 0xff, 0x00, 0x01, 0x32, 0x00, 0x18, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf2, 0x1c,
            /*00b0*/   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8,
            /*00c0*/   0x00, 0x2d, 0x31, 0x00, 0x18, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf3, 0x1c, 0x00, 0x00, 0x00, 0x00,
            /*00d0*/   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x00, 0x2d, 0x02, 0x00,
            /*00e0*/   0x08, 0x00, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x58, 0x00, 0x04, 0x00, 0x3f, 0x0c,
            /*00f0*/   0x00, 0x00, 0x62, 0x00, 0x14, 0x00, 0x10, 0x00, 0x00, 0x00, 0x50, 0x61, 0x72, 0x74, 0x69, 0x63,
            /*0100*/   0x69, 0x70, 0x61, 0x6e, 0x74, 0x5f, 0x70, 0x75, 0x62, 0x00, 0x01, 0x00, 0x00, 0x00
    };
    ProtocolVersion_t expectedVersion{2,1};
    VendorId_t expectedVendorID{1, 15};
    Guid expectedGuid{{0x01, 0x0f, 0x00, 0x2d, 0x9f, 0x7e, 0,0,0,0,0,0},{{0,0,1}, rtps::EntityKind_t::BUILD_IN_PARTICIPANT}};
    Locator expectedMetaMultiLoc = Locator::createUDPv4Locator(239,255,0,1, 7400);
    Locator expectedMetaUniLoc = Locator::createUDPv4Locator(192,168,0,45, 7410);
    Locator expectedDefaultUniLoc = Locator::createUDPv4Locator(192,168,0,45, 7411);
    Duration_t leaseDuration{-1, 0xffffffff};
    BuiltinEndpointSet_t expectedEndpoints{0xc3f};
    std::string expectedName{"Participant_pub"};

    ParticipantProxyData proxy;
    ucdrBuffer buffer;
    void SetUp() override{
        buffer.endianness = UCDR_LITTLE_ENDIANNESS;
        ucdr_init_buffer(&buffer, parameterListFromEprosima, sizeof(parameterListFromEprosima)/sizeof(parameterListFromEprosima[0]));
    }
};

TEST_F(ParticipantProxy, returnTrueOnValidPacket){

    bool success = proxy.readFromUcdrBuffer(buffer);
    EXPECT_TRUE(success);
}

TEST_F(ParticipantProxy, readsProtocolVersionCorrectly){

    proxy.readFromUcdrBuffer(buffer);

    EXPECT_EQ(proxy.m_protocolVersion.major, expectedVersion.major);
    EXPECT_EQ(proxy.m_protocolVersion.minor, expectedVersion.minor);
}

TEST_F(ParticipantProxy, readsProtocolVendorIdCorrectly){

    proxy.readFromUcdrBuffer(buffer);

    EXPECT_EQ(proxy.m_vendorId.vendorId, expectedVendorID.vendorId);
}

TEST_F(ParticipantProxy, readsGUIDCorrectly){

    proxy.readFromUcdrBuffer(buffer);

    EXPECT_EQ(proxy.m_guid.prefix.id, expectedGuid.prefix.id);
    EXPECT_EQ(proxy.m_guid.entityId.entityKey, expectedGuid.entityId.entityKey);
    EXPECT_EQ(proxy.m_guid.entityId.entityKind, expectedGuid.entityId.entityKind);
}

TEST_F(ParticipantProxy, readsMetaMuliCastLocatorCorrectly){

    proxy.readFromUcdrBuffer(buffer);

    EXPECT_EQ(proxy.m_metatrafficMulticastLocatorList[0].kind, expectedMetaMultiLoc.kind);
    EXPECT_EQ(proxy.m_metatrafficMulticastLocatorList[0].address, expectedMetaMultiLoc.address);
    EXPECT_EQ(proxy.m_metatrafficMulticastLocatorList[0].port, expectedMetaMultiLoc.port);
}

TEST_F(ParticipantProxy, readsBuiltInEndpointSetCorrectly){

    proxy.readFromUcdrBuffer(buffer);

    EXPECT_EQ(proxy.m_availableBuiltInEndpoints, expectedEndpoints);
}
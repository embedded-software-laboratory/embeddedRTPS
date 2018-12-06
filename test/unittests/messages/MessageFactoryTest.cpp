/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "BufferMock.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/common/types.h"

#include <array>
#include <rtps/config.h>


class MessageFactory_addHeader : public ::testing::Test{
protected:
    BufferMock buffer;

    const int PROTOCOL_NAME_START = 0;
    const int PROTOCOL_VERSION_START = 4;
    const int VENCOR_ID_START = 6;
    const int GUID_PREFIX_START = 8;
    const rtps::GuidPrefix_t someGUIDPrefix = {0,1,2,3,4,5,6,7,8,9,10,11};

    void SetUp() override{
        rtps::MessageFactory::addHeader(buffer, someGUIDPrefix);
    }
};


TEST_F(MessageFactory_addHeader, AddsProtocolName){
    std::array<uint8_t, 4> expected{'R', 'T', 'P', 'S'};

    buffer.hasElementsAt(expected, PROTOCOL_NAME_START);
}

TEST_F(MessageFactory_addHeader, AddsProtocolVersion){
    std::array<uint8_t, 2> expected = {rtps::PROTOCOLVERSION.major,
                                       rtps::PROTOCOLVERSION.minor};

    buffer.hasElementsAt(expected, PROTOCOL_VERSION_START);
}

TEST_F(MessageFactory_addHeader, AddsVendorId){
    buffer.hasElementsAt(rtps::Config::VENDOR_ID.vendorId, VENCOR_ID_START);
}

TEST_F(MessageFactory_addHeader, AddsGUIDPrefix){
    buffer.hasElementsAt(someGUIDPrefix.id, GUID_PREFIX_START);
}


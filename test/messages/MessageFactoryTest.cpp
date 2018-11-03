/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "BufferMock.h"
#include "rtps/communication/PBufWrapper.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/types.h"

#include <array>


class MessageFactory_addHeader : public ::testing::Test{
protected:
    BufferMock buffer;

    void SetUp() override{
        rtps::MessageFactory::addHeader(buffer);
    }
};


TEST_F(MessageFactory_addHeader, AddsProtocolName){
    std::array<uint8_t, 4> expected{'R', 'T', 'P', 'S'};
    buffer.hasElementsAt(expected, 0);
}

TEST_F(MessageFactory_addHeader, AddsProtocolVersion){
    std::array<uint8_t, 2> expected = {rtps::PROTOCOLVERSION.major,
                                       rtps::PROTOCOLVERSION.minor};
    buffer.hasElementsAt(expected, 4);
}
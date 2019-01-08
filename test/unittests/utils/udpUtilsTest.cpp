/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/utils/udpUtils.h"

using rtps::isUserPort;
using rtps::getBuiltInMulticastPort;
using rtps::getBuiltInUnicastPort;
using rtps::getUserMulticastPort;
using rtps::getUserUnicastPort;
using rtps::isMultiCastPort;

TEST(IsBuildinPort, WorksCorrecltyWithGetMethods){
    EXPECT_FALSE(isUserPort(getBuiltInMulticastPort()));
    EXPECT_FALSE(isUserPort(getBuiltInUnicastPort(1)));
    EXPECT_TRUE(isUserPort(getUserMulticastPort()));
    EXPECT_TRUE(isUserPort(getUserUnicastPort(1)));
}

TEST(IsMultiCast, WorksCorrecltyWithGetMethods){
    EXPECT_TRUE(isMultiCastPort(getBuiltInMulticastPort()));
    EXPECT_FALSE(isMultiCastPort(getBuiltInUnicastPort(1)));
    EXPECT_TRUE(isMultiCastPort(getUserMulticastPort()));
    EXPECT_FALSE(isMultiCastPort(getUserUnicastPort(1)));
}

TEST(GetParticipantIdFromPort, returnsInvalidIdIfPortIsNoValidParticipantPort){
    rtps::Ip4Port_t invalidPort = rtps::getUserUnicastPort(1) + static_cast<rtps::Ip4Port_t>(1);

    rtps::ParticipantId_t id = rtps::getParticipantIdFromUnicastPort(invalidPort, true);

    EXPECT_EQ(id, rtps::PARTICIPANT_ID_INVALID);
}

// Currently the check is commented out. Not sure if required. TODO check this
TEST(GetParticipantIdFromPort, DISABLED_returnsInvalidIdIfPortIsMultiCast){
    rtps::Ip4Port_t userMultiCastPort = rtps::getUserMulticastPort();
    rtps::Ip4Port_t builtinMultiCastPort = rtps::getBuiltInMulticastPort();

    rtps::ParticipantId_t id = rtps::getParticipantIdFromUnicastPort(userMultiCastPort, true);
    EXPECT_EQ(id, rtps::PARTICIPANT_ID_INVALID);

    rtps::ParticipantId_t id2 = rtps::getParticipantIdFromUnicastPort(builtinMultiCastPort, true);
    EXPECT_EQ(id2, rtps::PARTICIPANT_ID_INVALID);
}

TEST(GetParticipantIdFromPort, DISABLED_returnsValidIdforUserPort){
    // TODO
}

TEST(GetParticipantIdFromPort, DISABLED_returnsValidIdforNonUserPort){
    // TODO
}
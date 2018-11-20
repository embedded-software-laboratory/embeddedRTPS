/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/entities/Participant.h"
#include "rtps/types.h"

using rtps::Participant;

class SomeParticipant : public ::testing::Test{
protected:
    rtps::GuidPrefix_t somePrefix = {1};
    rtps::participantId_t someId = 1;
    Participant part{somePrefix, someId};
};

TEST_F(SomeParticipant, getNextUserEntityKey_increasesCorrectly){
    auto zeroKey = part.getNextUserEntityKey();

    ASSERT_THAT(zeroKey, testing::ElementsAre(0,0,0));

    auto nextKey = part.getNextUserEntityKey();

    ASSERT_THAT(nextKey, testing::ElementsAre(0,0,1));
}
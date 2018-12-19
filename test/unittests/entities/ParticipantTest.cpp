/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/entities/Participant.h"
#include "rtps/common/types.h"

#include "test/mocking/ReaderMock.h"

using rtps::Participant;

class SomeParticipant : public ::testing::Test{
protected:
    rtps::GuidPrefix_t somePrefix = {1};
    rtps::ParticipantId_t someId = 1;
    Participant part{somePrefix, someId};
};

TEST_F(SomeParticipant, getNextUserEntityKey_increasesCorrectly){
    auto zeroKey = part.getNextUserEntityKey();

    ASSERT_THAT(zeroKey, testing::ElementsAre(0,0,0));

    auto nextKey = part.getNextUserEntityKey();

    ASSERT_THAT(nextKey, testing::ElementsAre(0,0,1));
}

TEST_F(SomeParticipant, readerCanBeFoundAfterAdding){
    rtps::EntityId_t someEntityId{{1,2,3}, rtps::EntityKind_t::USER_DEFINED_READER_WITHOUT_KEY};
    ReaderMock mock{{somePrefix, someEntityId}};

    rtps::Reader* returnedReader = part.addReader(&mock);
    EXPECT_NE(returnedReader, nullptr);

    rtps::Reader* foundReader = part.getReader(someEntityId);
    EXPECT_EQ(foundReader, returnedReader);
}

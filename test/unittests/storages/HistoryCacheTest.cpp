/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include "rtps/types.h"
#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;
using rtps::CacheChange;
using rtps::SequenceNumber_t;

class HistoryTest : public ::testing::Test{
protected:
    SequenceNumber_t SNOne = {0,1};
    SequenceNumber_t SNTwo = {0,2};
    SequenceNumber_t SNThree = {1,1};

    CacheChange changeOne{rtps::ChangeKind_t::ALIVE, SNOne};
    CacheChange changeTwo{rtps::ChangeKind_t::ALIVE, SNTwo};
    CacheChange changeThree{rtps::ChangeKind_t::ALIVE, SNThree};
};

TEST_F(HistoryTest, ReturnsCorrectMinAndMaxSequenceNumber){

    HistoryCache history;
    history.addChange(std::move(changeOne));
    history.addChange(std::move(changeTwo));
    history.addChange(std::move(changeThree));

    EXPECT_EQ(history.getSeqNumMin(), SNOne);
    EXPECT_EQ(history.getSeqNumMax(), SNThree);
}

TEST_F(HistoryTest, getNextCacheChange){

    HistoryCache history;
    history.addChange(std::move(changeOne));
    const CacheChange* toRemove = history.addChange(std::move(changeTwo));
    history.addChange(std::move(changeThree));

    EXPECT_EQ(history.getNextCacheChange()->sequenceNumber, SNOne);
    EXPECT_EQ(history.getNextCacheChange()->sequenceNumber, SNTwo);
    EXPECT_EQ(history.getNextCacheChange()->sequenceNumber, SNThree);
    history.removeChange(toRemove);

    EXPECT_EQ(history.getNextCacheChange()->sequenceNumber, SNOne);
    EXPECT_EQ(history.getNextCacheChange()->sequenceNumber, SNThree);
}
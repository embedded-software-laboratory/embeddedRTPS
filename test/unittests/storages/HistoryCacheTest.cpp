/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include "rtps/common/types.h"
#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;
using rtps::CacheChange;
using rtps::SequenceNumber_t;


TEST(EmptyHistory, ReturnsUnknownSequenceNumberIfEmpty){
    HistoryCache history;
    EXPECT_EQ(history.getSeqNumMin(), rtps::SEQUENCENUMBER_UNKNOWN);
    EXPECT_EQ(history.getSeqNumMin(), rtps::SEQUENCENUMBER_UNKNOWN);
}


class HistoryWithThreeChanges : public ::testing::Test{
protected:
    HistoryCache history;

    SequenceNumber_t SNOne = {0,1};
    SequenceNumber_t SNTwo = {0,2};
    SequenceNumber_t SNThree = {1,1};

    CacheChange changeOne{rtps::ChangeKind_t::ALIVE, SNOne};
    CacheChange changeTwo{rtps::ChangeKind_t::ALIVE, SNTwo};
    CacheChange changeThree{rtps::ChangeKind_t::ALIVE, SNThree};

    void SetUp() override{
        history.addChange(std::move(changeOne));
        history.addChange(std::move(changeTwo));
        history.addChange(std::move(changeThree));
    }
};

TEST_F(HistoryWithThreeChanges, ReturnsCorrectMinAndMaxSequenceNumber){
    EXPECT_EQ(history.getSeqNumMin(), SNOne);
    EXPECT_EQ(history.getSeqNumMax(), SNThree);
}



TEST_F(HistoryWithThreeChanges, getChangeBySN_returnCorrectChange){

    auto change = history.getChangeBySN(changeTwo.sequenceNumber);

    ASSERT_NE(change, nullptr);
    EXPECT_EQ(change->sequenceNumber, SNTwo);
}

TEST_F(HistoryWithThreeChanges, getChangeBySN_cannotFindFirstAfter_dropFirst){
    history.dropFirst();

    auto change = history.getChangeBySN(SNOne);
    EXPECT_EQ(change, nullptr);
}

class HistoryWithFullBuffer : public ::testing::Test{
protected:
    HistoryCache history;
    const SequenceNumber_t firstSN = {0,1};
    SequenceNumber_t currentSN = firstSN;

    void SetUp() override{
        while(!history.isFull()){
            const CacheChange* change = history.addChange(CacheChange{rtps::ChangeKind_t::ALIVE, currentSN++});
            ASSERT_NE(change, nullptr);
        }
    }
};

TEST_F(HistoryWithFullBuffer, canFindAllSNs){
    SequenceNumber_t toFind(firstSN);
    while(toFind != currentSN){
        const CacheChange* change = history.getChangeBySN(toFind++);
        EXPECT_NE(change, nullptr);
    }
}

TEST_F(HistoryWithFullBuffer, canFindSNsAfterAddingMore){
    const int numSNsToCheck = 100;
    for(int i=0; i < numSNsToCheck; i++){
        const CacheChange* addedChange = history.addChange(CacheChange{rtps::ChangeKind_t::ALIVE, currentSN});
        ASSERT_NE(addedChange, nullptr);

        const CacheChange* foundChange = history.getChangeBySN(currentSN);
        EXPECT_NE(foundChange, nullptr);

        ++currentSN;
    }
}
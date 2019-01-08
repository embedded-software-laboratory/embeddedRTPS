/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/entities/WriterProxy.h"

class AWriterProxy : public ::testing::Test{
protected:
    rtps::Guid someGuid{{0}, {{1,2,3}, rtps::EntityKind_t::USER_DEFINED_WRITER_WITH_KEY}};
    rtps::Locator someLocator = rtps::Locator::createUDPv4Locator(1,2,3,4,5);

    rtps::WriterProxy proxy{someGuid, someLocator};
};

TEST_F(AWriterProxy, startsWithSequenceNumberOne){
    rtps::SequenceNumber_t expectedSN{0,1};

    EXPECT_EQ(proxy.expectedSN, expectedSN);
}

TEST_F(AWriterProxy, getMissingChanges_returnsAllFromFirstMissing){
    proxy.expectedSN = {0,2};
    rtps::SequenceNumber_t firstAvail{0,0};
    rtps::SequenceNumber_t lastAvail{0,52};

    rtps::SequenceNumberSet set = proxy.getMissing(firstAvail, lastAvail);

    for(uint32_t bit=proxy.expectedSN.low; bit <= lastAvail.low; ++bit){
        EXPECT_TRUE(set.isSet(bit));
    }
}

TEST_F(AWriterProxy, getNextCount_deliversIncreasingNumbers){
    rtps::Count_t first = proxy.getNextCount();
    rtps::Count_t second = proxy.getNextCount();

    EXPECT_EQ(first.value + 1, second.value);
}
/*
 * TDD was used here. Tests can be seen as example usage. There are no more features than listed here.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <rtps/entities/StatelessWriter.h>

using namespace rtps;

class EmptyRTPSWriterWithoutKey : public ::testing::Test{
protected:
    StatelessWriter writer{TopicKind_t::NO_KEY};

};

TEST_F(EmptyRTPSWriterWithoutKey, StartsWithSequenceNumberZero){
    SequenceNumber_t expectedResult{0,0};
    EXPECT_EQ(writer.getLastSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriterWithoutKey, AddChangeIncreasesSequenceNumber){
    SequenceNumber_t expectedResult {0,1};

    writer.newChange(ChangeKind_t::ALIVE, nullptr, 0);

    EXPECT_EQ(writer.getLastSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriterWithoutKey, AddChangeReturnsCorrectChange){
    ChangeKind_t expectedKind = ChangeKind_t::ALIVE;
    const data_size_t size = 5;
    uint8_t data[size] = {};

    CacheChange change = writer.newChange(expectedKind, data, size);

    EXPECT_EQ(change.kind, expectedKind);
    EXPECT_EQ(change.data, data);
    EXPECT_EQ(change.size, size);
}

TEST_F(EmptyRTPSWriterWithoutKey, IgnoresOnlyAddsKindAlive){
    SequenceNumber_t current = writer.getLastSequenceNumber();

    ChangeKind_t irrelevantKinds[] = {ChangeKind_t::INVALID,
                           ChangeKind_t::NOT_ALIVE_DISPOSED,
                           ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : irrelevantKinds){
        CacheChange change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change.kind, ChangeKind_t::INVALID);
        EXPECT_EQ(current, writer.getLastSequenceNumber());
    }
}

class EmptyRTPSWriterWithKey : public ::testing::Test{
protected:
    StatelessWriter writer{TopicKind_t::WITH_KEY};

};

TEST_F(EmptyRTPSWriterWithKey, IgnoresIgnoresInvalid){
    SequenceNumber_t current = writer.getLastSequenceNumber();

    CacheChange change = writer.newChange(ChangeKind_t::INVALID, nullptr, 0);

    EXPECT_EQ(change.kind, ChangeKind_t::INVALID);
    EXPECT_EQ(current, writer.getLastSequenceNumber());

}
TEST_F(EmptyRTPSWriterWithKey, AddsAllBesideInvalid){
    ChangeKind_t relevantKinds[] = {ChangeKind_t::ALIVE,
                                    ChangeKind_t::NOT_ALIVE_DISPOSED,
                                    ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : relevantKinds){
        SequenceNumber_t expected = ++writer.getLastSequenceNumber();

        CacheChange change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change.kind, kind);
        EXPECT_EQ(writer.getLastSequenceNumber(), expected);
    }

}






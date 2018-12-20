/*
 * TDD was used here. Tests can be seen as example usage. There are no more features than listed here.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/entities/StatelessWriter.h"
#include "rtps/rtps.h"

#include "test/CustomMatcher.h"

using namespace rtps;

class EmptyRTPSWriter : public ::testing::Test{
protected:
    const TopicKind_t arbitraryType = TopicKind_t::NO_KEY;
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData attributes{{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN},
                                          ReliabilityKind_t::BEST_EFFORT,
                                          getUserUnicastLocator(someParticipantId)};

    UdpDriver transport{nullptr, nullptr};
    StatelessWriter writer;
    static const DataSize_t size = 5;
    const uint8_t data[size] = {0, 1, 2, 3, 4};

    void SetUp() override{
        rtps::init();

        writer.init(attributes, arbitraryType, nullptr, transport);
    }
};

TEST_F(EmptyRTPSWriter, StartsWithSequenceNumberZero){
    SequenceNumber_t expectedResult{0,0};
    EXPECT_EQ(writer.getLastSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriter, newChange_IncreasesSequenceNumber){
    SequenceNumber_t expectedResult {0,1};

    writer.newChange(ChangeKind_t::ALIVE, nullptr, 0);

    EXPECT_EQ(writer.getLastSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriter, newChange_ReturnsChange){
    ChangeKind_t expectedKind = ChangeKind_t::ALIVE;

    const CacheChange* change = writer.newChange(expectedKind, data, size);

    EXPECT_NE(change, nullptr);
}

TEST_F(EmptyRTPSWriter, newChange_SetCorrectValues){
    ChangeKind_t expectedKind = ChangeKind_t::ALIVE;
    const DataSize_t size = 5;
    uint8_t data[size] = {};

    const CacheChange* change = writer.newChange(expectedKind, data, size);

    EXPECT_EQ(change->kind, expectedKind);
    EXPECT_EQ(change->sequenceNumber, writer.getLastSequenceNumber());
    EXPECT_THAT(change->data, PBufContains(data, size));
}

TEST_F(EmptyRTPSWriter, newChange_AllocatesExactSize){
    const DataSize_t size = 5;
    uint8_t data[size] = {};

    const CacheChange* change = writer.newChange(ChangeKind_t::ALIVE, data, size);

    EXPECT_EQ(change->data.firstElement->tot_len, size);
}


class EmptyRTPSWriterWithoutKey : public ::testing::Test{
protected:
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData attributes{{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN},
                                      ReliabilityKind_t::BEST_EFFORT,
                                      getUserUnicastLocator(someParticipantId)};

    UdpDriver transport{nullptr, nullptr};
    StatelessWriter writer;
    static const DataSize_t size = 5;
    const uint8_t data[size] = {0, 1, 2, 3, 4};

    void SetUp() override{
        rtps::init();
        writer.init(attributes, TopicKind_t::NO_KEY, nullptr, transport);
    }
};

TEST_F(EmptyRTPSWriterWithoutKey, newChange_IgnoresAllKindThatAreNotAlive){
    SequenceNumber_t current = writer.getLastSequenceNumber();

    ChangeKind_t irrelevantKinds[] = {ChangeKind_t::INVALID,
                           ChangeKind_t::NOT_ALIVE_DISPOSED,
                           ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : irrelevantKinds){
        const CacheChange* change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change, nullptr);
        EXPECT_EQ(current, writer.getLastSequenceNumber());
    }
}

class EmptyRTPSWriterWithKey : public ::testing::Test{
protected:
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData attributes{{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN},
                                      ReliabilityKind_t::BEST_EFFORT,
                                      getUserUnicastLocator(someParticipantId)};

    UdpDriver transport{nullptr, nullptr};
    StatelessWriter writer;
    void SetUp() override{
        writer.init(attributes, TopicKind_t::WITH_KEY, nullptr, transport);
    };
};

TEST_F(EmptyRTPSWriterWithKey, newChange_IgnoresKindInvalid){
    SequenceNumber_t current = writer.getLastSequenceNumber();

    const CacheChange* change = writer.newChange(ChangeKind_t::INVALID, nullptr, 0);

    EXPECT_EQ(change, nullptr);
    EXPECT_EQ(current, writer.getLastSequenceNumber());

}
TEST_F(EmptyRTPSWriterWithKey, newChange_AddsAllKindsBesideInvalid){
    ChangeKind_t relevantKinds[] = {ChangeKind_t::ALIVE,
                                    ChangeKind_t::NOT_ALIVE_DISPOSED,
                                    ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : relevantKinds){
        SequenceNumber_t expected = ++writer.getLastSequenceNumber();

        const CacheChange* change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change->kind, kind);
        EXPECT_EQ(writer.getLastSequenceNumber(), expected);
    }
}








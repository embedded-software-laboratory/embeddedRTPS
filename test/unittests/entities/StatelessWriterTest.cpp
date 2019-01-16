/*
 * TDD was used here. Tests can be seen as example usage. There are no more features than listed here.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/entities/StatelessWriter.h"
#include "rtps/rtps.h"

#include "test/CustomMatcher.h"
#include "test/globals.h"
#include "test/mocking/NetworkDriverMock.h"

using namespace rtps;

class EmptyRTPSWriter : public ::testing::Test{
protected:
    const TopicKind_t arbitraryType = TopicKind_t::NO_KEY;
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData attributes{TestGlobals::someWriterGuid,
                                          ReliabilityKind_t::BEST_EFFORT,
                                          getUserUnicastLocator(someParticipantId)};

    NetworkDriverMock transport;
    StatelessWriterT<NetworkDriverMock> writer;
    static const DataSize_t size = 5;
    const uint8_t data[size] = {0, 1, 2, 3, 4};

    void SetUp() override{
        rtps::init();
        writer.init(attributes, arbitraryType, nullptr, transport);
    }
};

TEST_F(EmptyRTPSWriter, StartsWithSequenceNumberZero){
    SequenceNumber_t expectedResult{0,0};
    EXPECT_EQ(writer.getLastUsedSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriter, newChange_IncreasesSequenceNumber){
    SequenceNumber_t expectedResult {0,1};

    writer.newChange(ChangeKind_t::ALIVE, nullptr, 0);

    EXPECT_EQ(writer.getLastUsedSequenceNumber(), expectedResult);
}

TEST_F(EmptyRTPSWriter, newChange_ReturnsCorrectChange){
    ChangeKind_t expectedKind = ChangeKind_t::ALIVE;

    const CacheChange* change = writer.newChange(expectedKind, data, size);

    ASSERT_NE(change, nullptr);
    //EXPECT_EQ(change->data.firstElement->tot_len, size);
}

TEST_F(EmptyRTPSWriter, newChange_SetCorrectValues){
    ChangeKind_t expectedKind = ChangeKind_t::ALIVE;
    std::array<uint8_t, 5> data{};

    const CacheChange* change = writer.newChange(expectedKind, data.data(), data.size());

    EXPECT_EQ(change->kind, expectedKind);
    EXPECT_EQ(change->sequenceNumber, writer.getLastUsedSequenceNumber());
    EXPECT_THAT(change->data, PBufContains(data));
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
    const BuiltInTopicData attributes{TestGlobals::someWriterGuid,
                                      ReliabilityKind_t::BEST_EFFORT,
                                      getUserUnicastLocator(someParticipantId)};

    NetworkDriverMock transport;
    StatelessWriterT<NetworkDriverMock> writer;
    static const DataSize_t size = 5;
    const uint8_t data[size] = {0, 1, 2, 3, 4};

    void SetUp() override{
        rtps::init();
        writer.init(attributes, TopicKind_t::NO_KEY, nullptr, transport);
    }
};

TEST_F(EmptyRTPSWriterWithoutKey, newChange_IgnoresAllKindThatAreNotAlive){
    SequenceNumber_t current = writer.getLastUsedSequenceNumber();

    ChangeKind_t irrelevantKinds[] = {ChangeKind_t::INVALID,
                           ChangeKind_t::NOT_ALIVE_DISPOSED,
                           ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : irrelevantKinds){
        const CacheChange* change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change, nullptr);
        EXPECT_EQ(current, writer.getLastUsedSequenceNumber());
    }
}

class EmptyRTPSWriterWithKey : public ::testing::Test{
protected:
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData attributes{TestGlobals::someWriterGuid,
                                      ReliabilityKind_t::BEST_EFFORT,
                                      getUserUnicastLocator(someParticipantId)};

    NetworkDriverMock transport;
    StatelessWriterT<NetworkDriverMock> writer;
    void SetUp() override{
        writer.init(attributes, TopicKind_t::WITH_KEY, nullptr, transport);
    };
};

TEST_F(EmptyRTPSWriterWithKey, newChange_IgnoresKindInvalid){
    SequenceNumber_t current = writer.getLastUsedSequenceNumber();

    const CacheChange* change = writer.newChange(ChangeKind_t::INVALID, nullptr, 0);

    EXPECT_EQ(change, nullptr);
    EXPECT_EQ(current, writer.getLastUsedSequenceNumber());

}
TEST_F(EmptyRTPSWriterWithKey, newChange_AddsAllKindsBesideInvalid){
    ChangeKind_t relevantKinds[] = {ChangeKind_t::ALIVE,
                                    ChangeKind_t::NOT_ALIVE_DISPOSED,
                                    ChangeKind_t::NOT_ALIVE_UNREGISTERED};
    for(auto kind : relevantKinds){
        SequenceNumber_t expected = ++writer.getLastUsedSequenceNumber();

        const CacheChange* change = writer.newChange(kind, nullptr, 0);
        EXPECT_EQ(change->kind, kind);
        EXPECT_EQ(writer.getLastUsedSequenceNumber(), expected);
    }
}

class FullRTPSWriterWithProxy : public ::testing::Test{
protected:
    const ParticipantId_t someParticipantId = 1;
    const BuiltInTopicData writerAttributes{TestGlobals::someWriterGuid,
                                      ReliabilityKind_t::BEST_EFFORT,
                                      getUserUnicastLocator(someParticipantId)};

    NetworkDriverMock transport;
    StatelessWriterT<NetworkDriverMock> writer;
    static const DataSize_t size = 5;
    const uint8_t data[size] = {0, 1, 2, 3, 4};

    void SetUp() override{
        writer.init(writerAttributes, TopicKind_t::WITH_KEY, nullptr, transport);
        writer.addNewMatchedReader(ReaderProxy(TestGlobals::someReaderGuid, TestGlobals::someUserUnicastLocator));
        for(int i=0; i < Config::HISTORY_SIZE; i++){
            const CacheChange* change = writer.newChange(ChangeKind_t::ALIVE, data, size);
            ASSERT_NE(change, nullptr);
        }
    };
};

TEST_F(FullRTPSWriterWithProxy, progess_sendsMessageAfterAddingOneMore){
    const CacheChange* change = writer.newChange(ChangeKind_t::ALIVE, data, size);
    ASSERT_NE(change, nullptr);

    EXPECT_CALL(transport, sendFunction);

    writer.progress();
}

TEST_F(FullRTPSWriterWithProxy, progess_sendsAllMessagesAfterSendingOneAndAddingANewOne){
    EXPECT_CALL(transport, sendFunction).Times(1);
    writer.progress();
    testing::Mock::VerifyAndClearExpectations(&writer); // This and EXPECT_CALL are used to avoid warning about uninteresting call

    const CacheChange* change = writer.newChange(ChangeKind_t::ALIVE, data, size);
    ASSERT_NE(change, nullptr);

    EXPECT_CALL(transport, sendFunction).Times(Config::HISTORY_SIZE);

    const int randomValueToCallMoreThanNecessary = 5;
    for(int i=0; i < Config::HISTORY_SIZE + randomValueToCallMoreThanNecessary; i++){
        writer.progress();
    }
}


/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include "test/mocking/BufferMock.h"
#include "test/mocking/ReaderMock.h"
#include "test/mocking/WriterMock.h"

#include "rtps/messages/MessageFactory.h"
#include "rtps/messages/MessageReceiver.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Participant.h"
#include "rtps/common/types.h"

using ::testing::_;


class AMessageReceiver : public ::testing::Test{
protected:
    rtps::GuidPrefix_t receiverGuidPrefix{1};
    rtps::ParticipantId_t someID = 0;
    rtps::Participant part{receiverGuidPrefix, someID};
    rtps::MessageReceiver* p_receiver;
    rtps::Header validHeader;

    void SetUp() override{
        p_receiver = part.getMessageReceiver();

        validHeader.protocolName = {'R', 'T', 'P', 'S'};
        validHeader.guidPrefix = {2};
        validHeader.vendorId = rtps::VENDOR_UNKNOWN;
        validHeader.protocolVersion = rtps::PROTOCOLVERSION_2_2;
    };
};

TEST_F(AMessageReceiver, processMessage_returnsTrueIfValidHeader){
    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_TRUE(valid);
}

TEST_F(AMessageReceiver, processMessage_setsInfoCorrectIfValidHeader){
    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_TRUE(valid);
    EXPECT_EQ(p_receiver->sourceGuidPrefix.id, validHeader.guidPrefix.id);
    EXPECT_EQ(p_receiver->sourceVendor.vendorId, validHeader.vendorId.vendorId);
    EXPECT_EQ(p_receiver->sourceVersion.major, validHeader.protocolVersion.major);
    EXPECT_EQ(p_receiver->sourceVersion.minor, validHeader.protocolVersion.minor);
    EXPECT_FALSE(p_receiver->haveTimeStamp);

}

TEST_F(AMessageReceiver, processMessage_returnsFalseIfInvalidProtocolType){
    validHeader.protocolName = {'R', 'T', 'P', '2'};

    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_FALSE(valid);
}

/**
 * This is the current state where we don't care about backward compatibility
 */
TEST_F(AMessageReceiver, processMessage_returnsFalseIfMajorVersionLower){
    // A lower major version is used because the header structure is only allowed to change
    // together with a new major version
    validHeader.protocolVersion = rtps::PROTOCOLVERSION_1_1;

    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_FALSE(valid);
}

TEST_F(AMessageReceiver, processMessage_returnsFalseIfMajorVersionIsHigher){
    // A lower major version is used because the header structure is only allowed to change
    // together with a new major version
    validHeader.protocolVersion = {3, 0};

    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_FALSE(valid);
}

TEST_F(AMessageReceiver, processMessage_returnsFalseIfItsOwnPackage){
    validHeader.guidPrefix = receiverGuidPrefix;

    auto data = reinterpret_cast<uint8_t*>(&validHeader);
    rtps::DataSize_t size = sizeof(validHeader);

    bool valid = p_receiver->processMessage(data, size);

    EXPECT_FALSE(valid);
}

class AMessageReceiverReceivedDataSubmessage : public ::testing::Test{
protected:
    ReaderMock correctReaderMock{{rtps::GUIDPREFIX_UNKNOWN, rtps::ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER}};
    ReaderMock anotherReaderMock{{rtps::GUIDPREFIX_UNKNOWN, rtps::ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER}};
    BufferMock validDataMsgBufferMock;
    rtps::GuidPrefix_t somePrefix = {1,2,3,4};
    BufferMock somePayload;

    rtps::GuidPrefix_t receiverGuidPrefix{1};
    rtps::ParticipantId_t someID = 0;
    rtps::Participant part{receiverGuidPrefix, someID};
    rtps::MessageReceiver receiver{&part};


    void SetUp() override{
        rtps::MessageFactory::addHeader(validDataMsgBufferMock, somePrefix);
        rtps::MessageFactory::addSubMessageData(validDataMsgBufferMock, somePayload, false, rtps::SequenceNumber_t{0,1},
                                                rtps::ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER,
                                                rtps::ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER);
    };
};

TEST_F(AMessageReceiverReceivedDataSubmessage, processMessage_validDataSMAddsCacheChangeToCorrectReader){
    part.addReader(&anotherReaderMock);
    part.addReader(&correctReaderMock);
    auto data = validDataMsgBufferMock.buffer.data();
    auto size = (rtps::DataSize_t) validDataMsgBufferMock.buffer.size();

    EXPECT_CALL(correctReaderMock, newChange(_)).Times(1);
    EXPECT_CALL(anotherReaderMock, newChange(_)).Times(0);

    bool valid = receiver.processMessage(data, size);

    EXPECT_TRUE(valid);
}

class AMessageReceiverReceivedHeartbeatSubmessage : public ::testing::Test{
protected:
    ReaderMock correctReaderMock{{rtps::GUIDPREFIX_UNKNOWN, rtps::ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER}};
    ReaderMock anotherReaderMock{{rtps::GUIDPREFIX_UNKNOWN, rtps::ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER}};
    BufferMock validHeartbeatMsgBufferMock;
    rtps::GuidPrefix_t somePrefix = {1,2,3,4};

    rtps::GuidPrefix_t receiverGuidPrefix{1};
    rtps::ParticipantId_t someID = 0;
    rtps::Participant part{receiverGuidPrefix, someID};
    rtps::MessageReceiver* p_receiver;


    void SetUp() override{
        part.addReader(&anotherReaderMock);
        part.addReader(&correctReaderMock);

        p_receiver = part.getMessageReceiver();

        rtps::MessageFactory::addHeader(validHeartbeatMsgBufferMock, somePrefix);
        rtps::MessageFactory::addHeartbeat(validHeartbeatMsgBufferMock, rtps::ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER,
                rtps::ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER, rtps::SequenceNumber_t{0,1}, rtps::SequenceNumber_t{0,5},
                rtps::Count_t{0});
    };
};

TEST_F(AMessageReceiverReceivedHeartbeatSubmessage, processMessage_callsCorrectFunction){
    auto data = validHeartbeatMsgBufferMock.buffer.data();
    auto size = (rtps::DataSize_t) validHeartbeatMsgBufferMock.buffer.size();

    EXPECT_CALL(correctReaderMock, onNewHeartbeat(_,_)).Times(1);
    p_receiver->processMessage(data, size);
}
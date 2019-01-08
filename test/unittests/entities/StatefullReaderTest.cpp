/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/entities/StatefullReader.h"
#include "rtps/messages/MessageTypes.h"
#include "test/mocking/NetworkDriverMock.h"

using namespace rtps;

void setVariableToTrue(void* success, ReaderCacheChange&){
    *static_cast<bool*>(success) = true; // using callee pointer for feedback
}

class StatefullReaderWithoutProxies : public ::testing::Test{
protected:
    const ParticipantId_t arbitraryParticipantId = 1;
    const Ip4Port_t srcPort = getUserUnicastPort(arbitraryParticipantId);
    NetworkDriverMock mockDriver;
    StatefullReaderT<NetworkDriverMock> reader;

    constexpr static DataSize_t dataSize = 5;
    uint8_t someData[dataSize] = {0};
    SequenceNumber_t nextSN = {0,1};
    Guid someGuid = {{1,1,1,1,1,1,1,1,1,1}, {{1,1,1}, EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    ReaderCacheChange someChange{ChangeKind_t::ALIVE, someGuid, nextSN, someData, dataSize};

    void SetUp() override{
        rtps::init();
        reader.init(Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}, mockDriver, srcPort);
    }
};

TEST_F(StatefullReaderWithoutProxies, newChange_doesntCallCallback){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.newChange(someChange);

    EXPECT_FALSE(success);
}

class StatefullReaderWithOneProxy : public ::testing::Test{
protected:
    const ParticipantId_t arbitraryParticipantId = 1;
    const Ip4Port_t srcPort = getUserUnicastPort(arbitraryParticipantId);
    NetworkDriverMock mockDriver;
    StatefullReaderT<NetworkDriverMock> reader;

    constexpr static DataSize_t dataSize = 5;
    uint8_t someData[dataSize] = {0};
    SequenceNumber_t nextSN = {0,1};
    Guid someGuid = {{1,1,1,1,1,1,1,1,1,1}, {{1,1,1}, EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    Guid anotherGuid = {{2,2,2,2,2,2,2,2,2,2,2,2}, {{2,2,2}, EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    Locator someLocator = Locator::createUDPv4Locator(1,2,3,4,5);
    ReaderCacheChange expectedCacheChange{ChangeKind_t::ALIVE, someGuid, nextSN, someData, dataSize};
    ReaderCacheChange cacheChangeFromUnknownWriter{ChangeKind_t::ALIVE, anotherGuid, nextSN, someData, dataSize};
    ReaderCacheChange cacheChangeWithDifferentSN{ChangeKind_t::ALIVE, someGuid, ++nextSN, someData, dataSize};

    WriterProxy someProxy{someGuid, someLocator};

    SubmessageHeartbeat hbMsg;

    void SetUp() override{
        rtps::init();
        reader.init(Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}, mockDriver, srcPort);
        someProxy.hbCount = {2};
        reader.addNewMatchedWriter(someProxy);

        hbMsg.header.submessageId = SubmessageKind::HEARTBEAT;
        hbMsg.header.submessageLength = sizeof(SubmessageHeartbeat) - MessageFactory::numBytesUntilEndOfLength;
        hbMsg.header.flags = FLAG_LITTLE_ENDIAN;
        // Force response by not setting final flag.
        hbMsg.writerId = someProxy.remoteWriterGuid.entityId;
        hbMsg.readerId = reader.m_guid.entityId;
        hbMsg.firstSN = SequenceNumber_t{0,1};
        hbMsg.lastSN = SequenceNumber_t{0,50};
        hbMsg.count = {someProxy.hbCount.value + 1};
    }
};

TEST_F(StatefullReaderWithOneProxy, newChange_callsCallbackIfCorrectSN){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.newChange(expectedCacheChange);

    EXPECT_TRUE(success);
}

TEST_F(StatefullReaderWithOneProxy, newChange_doesntAcceptSameSNTwiceForSameProxy){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.newChange(expectedCacheChange);
    ASSERT_TRUE(success);
    success = false;

    reader.newChange(expectedCacheChange);
    EXPECT_FALSE(success);
}

TEST_F(StatefullReaderWithOneProxy, newChange_doesntAcceptFromUnknownWriter){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.newChange(cacheChangeFromUnknownWriter);

    EXPECT_FALSE(success);
}

TEST_F(StatefullReaderWithOneProxy, newChange_doesntAcceptAfterRemovingWriter){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.removeWriter(someProxy.remoteWriterGuid);
    reader.newChange(expectedCacheChange);

    EXPECT_FALSE(success);
}

TEST_F(StatefullReaderWithOneProxy, newChange_doenstCallsCallbackIfNotRegistered){
    reader.newChange(expectedCacheChange);
    // Expect it doesn't try to call a nullptr
}


TEST_F(StatefullReaderWithOneProxy, onNewHeartBeat_sendsAckNack){
    EXPECT_CALL(mockDriver, sendFunction);

    reader.onNewHeartbeat(hbMsg, someProxy.remoteWriterGuid.prefix);
}

TEST_F(StatefullReaderWithOneProxy, onNewHeartBeat_isNotCalledWithInvalidMsg){
    EXPECT_CALL(mockDriver, sendFunction).Times(0);

    // Unknown GUID
    reader.onNewHeartbeat(hbMsg, anotherGuid.prefix);
    // Count already higher
    hbMsg.count = someProxy.hbCount;
    reader.onNewHeartbeat(hbMsg, someProxy.remoteWriterGuid.prefix);
}

TEST_F(StatefullReaderWithOneProxy, onNewHeartBeat_doesntAcceptSameMsgTwice){
    EXPECT_CALL(mockDriver, sendFunction).Times(1);
    reader.onNewHeartbeat(hbMsg, someProxy.remoteWriterGuid.prefix);
    reader.onNewHeartbeat(hbMsg, someProxy.remoteWriterGuid.prefix);
}

// TODO
TEST_F(StatefullReaderWithOneProxy, DISABLED_onNewHeartBeat_sendsValidAckNack){}
// TODO
TEST_F(StatefullReaderWithOneProxy, DISABLED_onNewHeartBeat_isThreadSafe){}

class StatefullReaderWithTwoProxy : public ::testing::Test{
protected:
    const ParticipantId_t arbitraryParticipantId = 1;
    const Ip4Port_t srcPort = getUserUnicastPort(arbitraryParticipantId);
    NetworkDriverMock mockDriver;
    StatefullReaderT<NetworkDriverMock> reader;

    constexpr static DataSize_t dataSize = 5;
    uint8_t someData[dataSize] = {0};
    SequenceNumber_t nextSN = {0,1};
    Guid someGuid = {{1,1,1,1,1,1,1,1,1,1}, {{1,1,1}, EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    Guid anotherGuid = {{2,2,2,2,2,2,2,2,2,2,2,2}, {{2,2,2}, EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    Locator someLocator = Locator::createUDPv4Locator(1,2,3,4,5);
    ReaderCacheChange firstProxyFirstCacheChange{ChangeKind_t::ALIVE, someGuid, nextSN, someData, dataSize};
    ReaderCacheChange secondProxyFirstCacheChange{ChangeKind_t::ALIVE, anotherGuid, nextSN, someData, dataSize};

    WriterProxy firstProxy{someGuid, someLocator};
    WriterProxy secondProxy{anotherGuid, someLocator};

    void SetUp() override{
        rtps::init();
        reader.init(Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}, mockDriver, srcPort);
        reader.addNewMatchedWriter(firstProxy);
        reader.addNewMatchedWriter(secondProxy);
    }
};

TEST_F(StatefullReaderWithTwoProxy, acceptSameSNForDifferentProxies){
    bool success = false;
    reader.registerCallback(setVariableToTrue, &success);

    reader.newChange(firstProxyFirstCacheChange);
    EXPECT_TRUE(success);
    success = false;
    reader.newChange(secondProxyFirstCacheChange);
    EXPECT_TRUE(success);
}

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/entities/StatefullReader.h"

using namespace rtps;

class AStatefullReader : public ::testing::Test{
protected:
    StatefullReader reader{Guid{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}};
    constexpr static DataSize_t dataSize = 5;
    uint8_t someData[dataSize] = {0};
    SequenceNumber_t nextSN{0,0};
    rtps::Guid someGuid = {{1,1,1,1,1,1,1,1,1,1}, {{1,1,1}, rtps::EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    rtps::Guid anotherGuid = {{2,2,2,2,2,2,2,2,2,2,2,2}, {{2,2,2}, rtps::EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    ReaderCacheChange expectedCacheChange{ChangeKind_t::ALIVE, someGuid, nextSN, someData, dataSize};

};

void callback(void* success, ReaderCacheChange&){
    *static_cast<bool*>(success) = true; // using callee pointer for feedback
}

TEST_F(AStatefullReader, newChange_callsCallbackIfExpectedSN){
    reader.createWriterProxy(someGuid);
    bool success = false;

    reader.registerCallback(callback, &success);
    reader.newChange(expectedCacheChange);

    EXPECT_TRUE(success);
}

TEST_F(AStatefullReader, newChange_doesntCallCallbackIfNoneRegistered){

    reader.newChange(expectedCacheChange);
}


TEST_F(AStatefullReader, getEmptyWriterProxy_returnsDifferentSlots){

    rtps::WriterProxy* proxy1 = reader.createWriterProxy(someGuid);
    rtps::WriterProxy* proxy2 = reader.createWriterProxy(anotherGuid);

    EXPECT_NE(proxy1, proxy2);
}

TEST_F(AStatefullReader, removeWriterProxy_deletesDependingOnGuid){
    /*rtps::WriterProxy* proxy1 =*/ reader.createWriterProxy(someGuid);
    rtps::WriterProxy* proxy2 = reader.createWriterProxy(anotherGuid);

    reader.removeWriter(anotherGuid);
    rtps::WriterProxy* proxy3 = reader.createWriterProxy(anotherGuid);
    EXPECT_EQ(proxy2, proxy3);
}

class AStatefullReaderWithNewWriterProxy : public ::testing::Test{
protected:
    StatefullReader reader{{GUIDPREFIX_UNKNOWN, ENTITYID_UNKNOWN}};
    constexpr static DataSize_t dataSize = 5;
    uint8_t someData[dataSize] = {0};
    SequenceNumber_t nextSN{0,0};
    rtps::Guid someGuid = {{1,2,3,4,5,6,7,8,9,10}, {{1,2,3}, rtps::EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY}};
    ReaderCacheChange firstCacheChange{ChangeKind_t::ALIVE, someGuid, nextSN, someData, dataSize};
    ReaderCacheChange secondCacheChange{ChangeKind_t::ALIVE, someGuid, ++nextSN, someData, dataSize};
    rtps::WriterProxy* proxy;

    void SetUp() override{
        proxy = reader.createWriterProxy(someGuid);
        proxy->init(someGuid);
    }
};

TEST_F(AStatefullReaderWithNewWriterProxy, newChange_dropsPacketIfNotExpectedSN){
    ReaderCacheChange unexpectedChange{ChangeKind_t::ALIVE, someGuid, ++nextSN, someData, dataSize};
    bool success = false;
    reader.registerCallback(callback, &success);

    reader.newChange(unexpectedChange);

    EXPECT_FALSE(success);
}

TEST_F(AStatefullReaderWithNewWriterProxy, newChange_acceptsChangesInCorrectOrder){
    bool success = false;
    reader.registerCallback(callback, &success);

    reader.newChange(firstCacheChange);
    EXPECT_TRUE(success);
    success = false;
    reader.newChange(secondCacheChange);
    EXPECT_TRUE(success);
}


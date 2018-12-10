/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/entities/Participant.h"
#include "test/mocking/WriterMock.h"
#include "test/mocking/ReaderMock.h"

using rtps::SEDPAgent;
using rtps::Participant;
using testing::_;

class AnSEDPAgent : public ::testing::Test{
protected:
    SEDPAgent agent;

    rtps::BuiltInEndpoints endpoints;
    WriterMock sedpPubWriter;
    WriterMock sedpSubWriter;
    ReaderMock sedpPubReader{rtps::ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER};
    ReaderMock sedpSubReader{rtps::ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER};

    void SetUp() override{
        endpoints.sedpSubWriter = &sedpSubWriter;
        endpoints.sedpPubWriter = &sedpPubWriter;
        endpoints.sedpSubReader = &sedpSubReader;
        endpoints.sedpPubReader = &sedpPubReader;
    }
};

TEST_F(AnSEDPAgent, initRegistersCallback){

    EXPECT_CALL(sedpSubReader, registerCallback(_,_)).Times(1);
    EXPECT_CALL(sedpPubReader, registerCallback(_,_)).Times(1);

    agent.init(endpoints);

}


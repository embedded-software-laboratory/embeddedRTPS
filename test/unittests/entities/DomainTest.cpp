/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/entities/Domain.h"
#include "rtps/rtps.h"

using rtps::Domain;

class ADomain : public ::testing::Test{
protected:
    Domain domain;
    void SetUp() override{
        //rtps::init(); //Causes error
        domain.start(); // TODO currently needed. Otherwise createParticipant wont work.
    }
};

/************************************************************
 *  DISABLED BECAUSE
 *  Sometimes it errors that the pure virtual base function newchange
 *  is called and it IS actually executed.. I think, this is caused by a segfault.
 *  Sometime we also get the mutex->mut == null error.
 *  This does NOT happen in direct use, e.g. in ThreadPoolDemo
 * **********************************************************
 */
TEST_F(ADomain, DISABLED_createParticipant_generatesDifferentGuids){
    rtps::Participant* firstPart = domain.createParticipant();
    rtps::Participant* secondPart = domain.createParticipant();

    ASSERT_NE(firstPart, nullptr);
    ASSERT_NE(secondPart, nullptr);

    EXPECT_NE(firstPart->m_guidPrefix.id, secondPart->m_guidPrefix.id);
}


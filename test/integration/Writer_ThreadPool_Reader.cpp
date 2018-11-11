/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "gtest/gtest.h"

#include "rtps/rtps.h"


class WTR : public ::testing::Test{
protected:
    static void SetUpTestCase(){
        rtps::init();
    }

    void SetUp() override{
        rtps::start();
    }

    void TearDown() override{
        rtps::stop();
    }
};

TEST_F(WTR, CreatesMessageFromCacheChangeAndSends){

}
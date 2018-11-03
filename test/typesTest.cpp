/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <rtps/types.h>
#include <limits>

using rtps::SequenceNumber_t;


TEST(SequenceNumber, IsInvalidAfterOverflow){
    SequenceNumber_t maxNumber{0, std::numeric_limits<uint32_t>::max()};
    SequenceNumber_t expected{1,0};

    ++maxNumber;

    EXPECT_EQ(maxNumber, expected);
}

// Considered this case, however it is not worth it. Too much computation every time
// for a case which should not happen as the variables are large enough.
//TEST(SequenceNumber_t, BecomesInvalidAfterOverflow){}


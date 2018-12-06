/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <rtps/common/types.h>
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


TEST(Time_t, Create_ComputesCorrectFractionFor100100100ns){
    uint32_t nanoseconds = 100100100; // 100ms, 100 mcs, 100ns

    rtps::Time_t time = rtps::Time_t::create(0, nanoseconds);

    constexpr auto twoPow32 = static_cast<uint64_t>(1) << 32;
    EXPECT_FLOAT_EQ(time.fraction/static_cast<float>(twoPow32), 0.100100100);

}


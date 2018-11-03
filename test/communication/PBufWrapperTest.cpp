/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <cstring>

#include "rtps/rtps.h"
#include "rtps/communication/PBufWrapper.h"

using rtps::PBufWrapper;

TEST(PBufWrapperGeneral, Ctor_AllocatesEnoughMemory){
    rtps::init();
    uint16_t size = 10;

    PBufWrapper wrapper{size};

    ASSERT_TRUE(wrapper.isValid());
    ASSERT_NE(wrapper.firstElement, nullptr);
    EXPECT_EQ(wrapper.firstElement->tot_len, size);

}

TEST(PBufWrapperRuntime, MultipleAllocByCtorAndFree){

    // TODO Use constant maximum amount of memory
    for(int i=1; i<1000; ++i){
        PBufWrapper wrapper(i);
    }
}

class EmptyPBufWrapper : public ::testing::Test{
protected:
    PBufWrapper wrapper;

    void SetUp() override{
        rtps::init();
    }
};

TEST_F(EmptyPBufWrapper, AppendData_FailsWithoutAllocatedMemory){
    uint8_t i = 2;

    bool success = wrapper.append(&i, 1);

    EXPECT_FALSE(success);
}

TEST_F(EmptyPBufWrapper, Reserve_CreatesNewPBuf){
    uint16_t size = 10;

    bool res = wrapper.reserve(size);

    ASSERT_TRUE(res);
    ASSERT_NE(wrapper.firstElement, nullptr);
    EXPECT_EQ(wrapper.firstElement->tot_len, size);
}

TEST_F(EmptyPBufWrapper, Reserve_DoesNotOverrideExisiting){
    wrapper.reserve(10);
    auto old = wrapper.firstElement;

    wrapper.reserve(5);

    ASSERT_EQ(wrapper.firstElement, old);

}

class WrapperWith10ByteReserved : public ::testing::Test{
protected:
    PBufWrapper wrapper;
    std::array<uint8_t, 10> data = {0,1,2,3,4,5,6,7,8,9};

    void SetUp() override{
        rtps::init();
        bool success = wrapper.reserve(10);
        ASSERT_TRUE(success);
        ASSERT_TRUE(wrapper.isValid());
    }
};

TEST_F(WrapperWith10ByteReserved, AppendData_CanAdd10Byte){

    bool success = wrapper.append(data.data(), data.size());

    ASSERT_TRUE(success);
    for(size_t i=0; i<data.size(); i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i]);
    }
}

TEST_F(WrapperWith10ByteReserved, AppendData_CanAdd5ByteTwoTimes){

    bool success1 = wrapper.append(data.data(), 5);
    bool success2 = wrapper.append(data.data(), 5);

    ASSERT_TRUE(success1);
    ASSERT_TRUE(success2);
    for(size_t i=0; i<10; i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i%5]);
    }
}

TEST_F(WrapperWith10ByteReserved, AppendData_CannotAdd10BytesAfterAdding5){
    wrapper.append(data.data(), 5);

    bool success = wrapper.append(data.data(), data.size());

    ASSERT_FALSE(success);
}

TEST_F(WrapperWith10ByteReserved, ReserveAppend_WorksWhenNotEnoughSpaceLeft){
    wrapper.append(data.data(), 5);

    bool successRes = wrapper.reserve(data.size());
    ASSERT_TRUE(successRes);
    bool successAppend= wrapper.append(data.data(), data.size());
    ASSERT_TRUE(successAppend);
}

TEST(PBufWrapperTest, AllocByCtorFillAndFree){
    const uint8_t data[]{0,1,2,3,4,5,6,7};
    uint16_t size = sizeof(data)/sizeof(data[0]);
    {
        PBufWrapper wrapper(size);
        bool success = wrapper.append(data, size);
        EXPECT_TRUE(success);
        for(int i=0; i<size; i++){
            EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i]);
        }
    }
}

class PBufWrapperWithTwoElementChain : public ::testing::Test{
protected:
    const uint8_t data[8]{0,1,2,3,4,5,6,7};
    const static uint16_t lengthData = sizeof(data)/sizeof(data[0]);
    const static uint16_t lengthFirst = 5;
    static_assert(lengthFirst < lengthData);
    const static uint16_t lengthSecond = lengthData - lengthFirst;
    PBufWrapper wrapper;
    const std::array<uint8_t, lengthFirst> first{};
    std::array<uint8_t, lengthSecond> second{};

    void SetUp() override{
        rtps::init();
        wrapper.reserve(lengthFirst); // reserve to get position of first empty byte correct
        wrapper.firstElement->tot_len += lengthFirst;
        wrapper.firstElement->next = new pbuf{nullptr, second.data(), lengthSecond, lengthSecond, 0, 1, 120, 0};
        std::memcpy(wrapper.firstElement->payload, first.data(), lengthFirst);
        std::memcpy(wrapper.firstElement->next->payload, second.data(), lengthSecond);
    }

    void TearDown() override{
        // Wrapper cannot clean up those
        delete wrapper.firstElement->next;
        wrapper.firstElement->next = nullptr;
    }
};

TEST_F(PBufWrapperWithTwoElementChain, Append_FillsBothCorrectly){

    bool success = wrapper.append(data, lengthData);

    EXPECT_TRUE(success);
    for(int i=0; i<lengthFirst; i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i]);
    }
    for(int i=0; i<lengthSecond; i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->next->payload)[i], data[lengthFirst + i]);
    }
}

TEST_F(PBufWrapperWithTwoElementChain, Append_DoesNotCopyTooMuchInFirstElement){
    const auto positionToCheck = lengthFirst - 1;
    ASSERT_EQ(first[positionToCheck], 0);
    ASSERT_NE(data[positionToCheck], 0);

    wrapper.append(data, positionToCheck);

    ASSERT_EQ(first[positionToCheck], 0);
}

TEST_F(PBufWrapperWithTwoElementChain, Append_DoesNotCopyTooMuchInSecondElement){
    const auto positionToCheck = lengthFirst + 1;
    ASSERT_EQ(second[positionToCheck - lengthFirst], 0);
    ASSERT_NE(data[positionToCheck], 0);

    wrapper.append(data, positionToCheck);

    ASSERT_EQ(second[positionToCheck - lengthFirst], 0);
}


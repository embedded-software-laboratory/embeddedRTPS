/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/rtps.h"
#include "rtps/communication/PBufWrapper.h"

using rtps::PBufWrapper;

class PBufWrapperTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        rtps::init();
    }
};

TEST_F(PBufWrapperTest, MultipleAllocAndFree){
    // TODO Use constant maximum amount of memory
    for(int i=1; i<10000; ++i){
        PBufWrapper wrapper(PBUF_TRANSPORT, i, PBUF_POOL);
    }
}

TEST_F(PBufWrapperTest, FailOnFillWithoutAllocation){
    uint8_t i = 2;
    PBufWrapper wrapper;
    bool success = wrapper.fillBuffer(&i, 1);
    EXPECT_FALSE(success);
}

TEST_F(PBufWrapperTest, AllocFillAndFree){
    const uint8_t data[]{'d','e','a','d','b','e','e','f'};
    uint16_t size = sizeof(data)/sizeof(data[0]);
    {
        PBufWrapper wrapper(PBUF_TRANSPORT, size, PBUF_POOL);
        bool success = wrapper.fillBuffer(data, size);
        EXPECT_TRUE(success);
        for(int i=0; i<size; i++){
            EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i]);
        }
    }
}

TEST_F(PBufWrapperTest, FillIntoMultipleChains){
    const uint8_t data[]{'d','e','a','d','b','e','e','f'};
    const uint16_t lengthData = sizeof(data)/sizeof(data[0]);
    const uint16_t lengthFirst = 5;
    static_assert(lengthFirst < lengthData);
    const uint16_t lengthSecond = lengthData - lengthFirst;
    // Construction of synthetic chains
    PBufWrapper wrapper;
    uint8_t first[lengthFirst];
    uint8_t second[lengthSecond];
    pbuf secondElement{NULL, &second, lengthSecond, lengthSecond, 0, 1, 120}; // Last three values taken from actual allocated pbuf
    pbuf firstElement{&secondElement, &first, lengthData, lengthFirst, 0, 1, 120};
    wrapper.firstElement = &firstElement;

    bool success = wrapper.fillBuffer(data, lengthData);

    EXPECT_TRUE(success);
    for(int i=0; i<lengthFirst; i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->payload)[i], data[i]);
    }
    for(int i=0; i<lengthSecond; i++){
        EXPECT_EQ(((uint8_t*)wrapper.firstElement->next->payload)[i], data[lengthFirst + i]);
    }
}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>

#include "rtps/storages/MemoryPool.h"

using rtps::MemoryPool;

class JustCreatedMemoryPool : public ::testing::Test{
protected:
    static const uint8_t SIZE = 10;
    MemoryPool<uint8_t, SIZE> pool;
    const uint8_t someData = 2;
};

TEST_F(JustCreatedMemoryPool, isEmpty){

    EXPECT_EQ(pool.getSize(), 10);
    EXPECT_FALSE(pool.isFull());
}

TEST_F(JustCreatedMemoryPool, loopWithPostIncrement_showZeroElements){
    uint8_t count = 0;

    for(auto it = pool.begin(); it != pool.end(); it++){
        count++;
    }

    EXPECT_EQ(count, 0);
}

TEST_F(JustCreatedMemoryPool, add_isSuccessfull){
    bool success = pool.add(someData);
    EXPECT_TRUE(success);
}

TEST_F(JustCreatedMemoryPool, rangeLoop_showsOneElemntAfterAdd){
    bool success = pool.add(someData);
    ASSERT_TRUE(success);

    uint8_t count = 0;

    for (auto &it : pool) {
        EXPECT_EQ(it, someData);
        count++;
    }

    EXPECT_EQ(count, 1);
}

class MemoryPoolWithThreeElements : public ::testing::Test{
protected:
    static const uint8_t SIZE = 10;
    MemoryPool<uint8_t, SIZE> pool;
    const uint8_t dataOne = 1;
    const uint8_t dataTwo = 2;
    const uint8_t dataThree = 3;

    void SetUp() override{
        bool success = true;
        success = success && pool.add(dataOne);
        success = success && pool.add(dataTwo);
        success = success && pool.add(dataThree);
        ASSERT_TRUE(success);
    }
};

TEST_F(MemoryPoolWithThreeElements, iterates_inCorrectOrder){
    auto it = pool.begin();
    EXPECT_EQ(*it, dataOne);
    EXPECT_EQ(*(++it), dataTwo);
    it++;
    EXPECT_EQ(*it++, dataThree);
    EXPECT_FALSE(it != pool.end());
}

TEST_F(MemoryPoolWithThreeElements, iterator_skipsDeletedValue){
    const uint8_t data = dataTwo;

    auto callback=[data](const uint8_t& value){return value == data;};
    auto thunk=[](void* arg, const uint8_t& value){return (*static_cast<decltype(callback)*>(arg))(value);};

    pool.remove(thunk, &callback);
    auto it = pool.begin();
    EXPECT_EQ(*it++, dataOne);
    EXPECT_EQ(*it++, dataThree);
    EXPECT_FALSE(it != pool.end());
}



/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <condition_variable>

#include "rtps/ThreadPool.h"
#include "rtps/common/types.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/config.h"
#include "rtps/rtps.h"
#include "test/mocking/WriterMock.h"

class ThreadPoolTest : public ::testing::Test{
protected:
    rtps::ThreadPool pool{nullptr, nullptr};

    void SetUp() override{
        rtps::init();
        bool success = pool.startThreads();
        ASSERT_TRUE(success);
    }

    void TearDown() override{
        pool.stopThreads();
    }
};

TEST_F(ThreadPoolTest, addWorkload_executesCallbackWithinHalfSecond){
    WriterMock mock;
    std::mutex m;
    std::condition_variable cond_var;
    bool done = false;

    EXPECT_CALL(mock, progress()).Times(2)
                                 .WillOnce(testing::Invoke([&]()->void {}))
                                 .WillOnce(testing::Invoke([&]()->void {
                                     std::unique_lock<std::mutex> lock(m);
                                     done = true;
                                     lock.unlock();
                                     cond_var.notify_one();
                                 }));

    pool.addWorkload(rtps::ThreadPool::Workload_t{&mock});
    pool.addWorkload(rtps::ThreadPool::Workload_t{&mock});

    std::unique_lock<std::mutex> lock(m);
    EXPECT_TRUE(cond_var.wait_for(lock, std::chrono::milliseconds(500), [&done] { return done; }));
}


class ThreadPool_WithReadCallback : public ::testing::Test{
protected:
    std::mutex mut;
    rtps::ThreadPool pool;
    std::condition_variable wasExecuted;
    unsigned int count{0};

    rtps::PacketInfo somePacket{};

    ThreadPool_WithReadCallback() : pool{receiveJumppad, this}{
    }

    static void receiveJumppad(void* callee, const rtps::PacketInfo& packet){
        auto me = static_cast<ThreadPool_WithReadCallback*>(callee);
        me->receiveFunction(packet);
    }

    void receiveFunction(const rtps::PacketInfo& /*packet*/){
        std::unique_lock<std::mutex> lock(mut);
        count++;
        wasExecuted.notify_all();
    }

    void waitForCountAndFailOnTimeout(unsigned int expectedCount, unsigned int maxNumDropped = 0, const std::chrono::milliseconds& time=std::chrono::milliseconds(100)){
        std::unique_lock<std::mutex> lock(mut);
        EXPECT_TRUE(wasExecuted.wait_for(lock, time, [&]{return expectedCount == count;}) ||
                    count >= expectedCount - maxNumDropped);
    }

    void SetUp() override{
        static_assert(rtps::Config::THREAD_POOL_NUM_READERS > 1, "We need at least 2 reader threads");
        rtps::init();
        bool success = pool.startThreads();
        ASSERT_TRUE(success);
    }

    void TearDown() override{
        pool.stopThreads();
        sys_msleep(100);
    }
};

TEST_F(ThreadPool_WithReadCallback, ProcessesOnePacket){
    unsigned int numberOfPackets{1};

    pool.addNewPacket(rtps::PacketInfo{});
    waitForCountAndFailOnTimeout(numberOfPackets);
}

TEST_F(ThreadPool_WithReadCallback, ProcessesAlmostAllPackets){
    unsigned int numberOfPackets{200};
    unsigned int maxNumDroppedPackets{50};
    unsigned int droppedPackets = 0;
    for(unsigned int i{0}; i < numberOfPackets; i++){
        if(!pool.addNewPacket(rtps::PacketInfo{})){
            droppedPackets++;
        }
        usleep(50);
    }
    waitForCountAndFailOnTimeout(numberOfPackets, maxNumDroppedPackets);
    EXPECT_EQ(count + droppedPackets, numberOfPackets);
}
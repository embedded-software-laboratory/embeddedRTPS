/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <condition_variable>

#include "rtps/ThreadPool.h"
#include "test/mocking/WriterMock.h"

class ThreadPoolTest : public ::testing::Test{
protected:
    rtps::ThreadPool pool;

    void SetUp() override{
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

    EXPECT_CALL(mock, createMessageCallback(::testing::_)).Times(1)
                                      .WillOnce(testing::Invoke([&](rtps::PBufWrapper&)->void {
                                          std::lock_guard<std::mutex> lock(m);
                                          done = true;
                                          cond_var.notify_one();
                                      }));
    pool.addWorkload(mock);

    std::unique_lock<std::mutex> lock(m);
    EXPECT_TRUE(cond_var.wait_for(lock, std::chrono::milliseconds(500), [&done] { return done; }));
}

//TEST_F(ThradPoolTest)


TEST(ThreadPool, sendReceive){
// TODO
/* Idea:
 * Use an UdpInterface mock which just calls receive on send.
 * The packet and the send should come from a mock-pair of Writer and Reader.
 * Problem: Injection of the interface in the driver.
 * Template and injection via constructor or function change threadpool class a lot.
 */
}
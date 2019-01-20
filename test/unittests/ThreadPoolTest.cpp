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
#include "rtps/rtps.h"
#include "rtps/entities/Domain.h"
#include "test/mocking/WriterMock.h"

class ThreadPoolTest : public ::testing::Test{
protected:
    rtps::Domain domain; // TODO Cause of crash
    rtps::ThreadPool pool{domain};
    ip4_addr_t someIp4Addr = {1234567};
    rtps::Ip4Port_t somepPort = 123;
    udp_pcb someUdpPcb;


    void SetUp() override{
        rtps::init();
        bool success = pool.startThreads();
        ASSERT_TRUE(success);
    }

    void TearDown() override{
        pool.stopThreads();
    }
};

TEST_F(ThreadPoolTest, DISABLED_addWorkload_executesCallbackWithinHalfSecond){
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
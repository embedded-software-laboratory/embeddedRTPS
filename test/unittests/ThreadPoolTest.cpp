/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <condition_variable>

#include "rtps/ThreadPool.h"
#include "rtps/types.h"
#include "rtps/entities/Domain.h"
#include "test/mocking/WriterMock.h"

class ThreadPoolTest : public ::testing::Test{
protected:
    rtps::Domain domain;
    rtps::ThreadPool pool{domain};
    ip4_addr_t someIp4Addr = {1234567};
    rtps::ip4_port_t somepPort = 123;
    udp_pcb someUdpPcb;


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

    EXPECT_CALL(mock, createMessageCallback(::testing::_)).Times(2)
                                      .WillOnce(testing::Invoke([&](rtps::ThreadPool::PacketInfo&)->bool {
                                          return false; // No message to send.
                                      }))
                                      .WillOnce(testing::Invoke([&](rtps::ThreadPool::PacketInfo&)->bool {
                                          std::lock_guard<std::mutex> lock(m);
                                          done = true;
                                          cond_var.notify_one();
                                          return false; // No message to send.
                                      }));

    pool.addWorkload(rtps::ThreadPool::Workload_t{&mock, 2});

    std::unique_lock<std::mutex> lock(m);
    EXPECT_TRUE(cond_var.wait_for(lock, std::chrono::milliseconds(500), [&done] { return done; }));
}
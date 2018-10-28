/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/UdpConnection.h"
#include "rtps/UdpDriver.h"
#include "LwipInterfaceMock.h"

#include <array>


TEST(UpdConnection, MoveSemantics){
    using UdpConnection = UdpConnectionT<LwipInterfaceMock>;

    std::array<UdpConnection, 5> conns;
    EXPECT_EQ(conns[0].pcb, nullptr);

    UdpConnection conn({42}, 666);
    udp_pcb *contentAddr = conn.pcb;
    conns[0] = std::move(conn);
    EXPECT_EQ(conns[0].pcb, contentAddr);
    EXPECT_EQ(conn.pcb, nullptr);
    EXPECT_EQ(conns[0].addr.addr, conn.addr.addr);
    EXPECT_EQ(conns[0].port, conn.port);
}

TEST(UdpDriver, FindExistingConnection){
    UdpDriver driver;

}

TEST(ThreadPool, sendReceive){
    // TODO
    /* Idea:
     * Use an UdpInterface mock which just calls receive on send.
     * The packet and the send should come from a mock-pair of Writer and Reader.
     * Problem: Injection of the interface in the driver.
     * Template and injection via constructor or function change threadpool class a lot.
     */
}
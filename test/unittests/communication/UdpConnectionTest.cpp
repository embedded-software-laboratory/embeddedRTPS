/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/rtps.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/communication/UdpConnection.h"
#include "rtps/communication/UdpDriver.h"
#include "LwipInterfaceMock.h"

#include <array>


TEST(UpdConnection, MoveSemantics){
    using UdpConnection = rtps::UdpConnectionT<LwipInterfaceMock>;

    std::array<UdpConnection, 5> conns;
    EXPECT_EQ(conns[0].pcb, nullptr);

    UdpConnection conn(666);
    udp_pcb *contentAddr = conn.pcb;
    conns[0] = std::move(conn);
    EXPECT_EQ(conns[0].pcb, contentAddr);
    EXPECT_EQ(conn.pcb, nullptr);
    EXPECT_EQ(conns[0].port, conn.port);
}

/*
 * This test file is used to get a better understanding of how LwIp works and defines behavior assumptions.
 * No claim to completeness.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */
#include <gtest/gtest.h>

#include "rtps/storages/PBufWrapper.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/rtps.h"
#include <lwip/udp.h>
#include <lwip/sys.h>
#include <lwipcfg.h>
#include <lwip/tcpip.h>
#include <cstring>

class LwIp : public ::testing::Test{
protected:
    void SetUp() override{
        rtps::init();
    }
};

TEST_F(LwIp, pcbAllocation){
    // TODO rtps::init needs to be called. Currently omitted because it's done in another test. Needs a better method
    for(int i=1; i < 10000; i++){
        pbuf* buff = pbuf_alloc(PBUF_TRANSPORT, i, PBUF_POOL);
        pbuf_free(buff);
    }
}

bool ready[2] = {false, false};
void writer(void* args){
    uint8_t id = *((int*) args);
    for(int i=1; i < 1000; i++){
        pbuf* buff = pbuf_alloc(PBUF_TRANSPORT, i, PBUF_POOL);
        static_cast<uint8_t*>(buff->payload)[0] = id;
        ASSERT_EQ(static_cast<uint8_t*>(buff->payload)[0], id);
        pbuf_free(buff);
    }
    ready[id] = true;
}

TEST_F(LwIp, pcbAllocationMultiThreaded){
    uint8_t first = 0;
    uint8_t second = 1;
    sys_thread_new("First", writer, &first, 10000, 3);
    sys_thread_new("Seconds", writer, &second, 10000, 3);
    while(!ready[0] || !ready[1]);
}


bool callbackFinished = false;
uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};

void receiveCallback(void*, udp_pcb*, pbuf* p, const ip_addr_t*, uint16_t){
    auto* receivedData = static_cast<uint8_t*>(p->payload);
    EXPECT_EQ(p->tot_len, p->len);
    for(int i=0; i< p->len; ++i){
        if(i < 8){
            EXPECT_EQ(data0[i], receivedData[i]);
        }else{
            EXPECT_EQ(data1[i%8], receivedData[i]);
        }
    }
    callbackFinished = true;
}

TEST_F(LwIp, SendSelfChainedPBufs){
    const uint16_t port = 7050;
    ip4_addr addr;
    LWIP_PORT_INIT_IPADDR(&addr); // self

    rtps::UdpDriver driver{receiveCallback, nullptr};


    pbuf* first = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);
    pbuf* second = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);

    EXPECT_EQ(second->ref, 1);

    first->next = second;
    first->tot_len = first->tot_len + second->len;
    second->ref++;

    std::memcpy(first->payload, data0, 8);
    std::memcpy(second->payload, data1, 8);
    rtps::PacketInfo packet;
    packet.destPort = port;
    packet.srcPort = port;
    packet.destAddr = addr;
    packet.buffer = rtps::PBufWrapper(first);
    driver.sendFunction(packet);

    sys_msleep(50);
    EXPECT_TRUE(callbackFinished);
}

TEST_F(LwIp, CombineAndSplitBehavior){
    pbuf* first  = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_POOL);
    pbuf* second = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_POOL);
    pbuf* third  = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_POOL);
    pbuf* fourth = pbuf_alloc(PBUF_TRANSPORT, 10, PBUF_POOL);

    pbuf_chain(first, second);
    pbuf_chain(first, third);
    pbuf_chain(first, fourth);

    EXPECT_EQ(first->tot_len, 40);

    pbuf* tail = pbuf_dechain(second);

    EXPECT_EQ(tail, third);
    EXPECT_EQ(first->tot_len, 40); // NOTE: THIS IS NOT ADJUSTED!!
    EXPECT_EQ(second->tot_len, 10);
    EXPECT_EQ(tail->tot_len, 20);


}

void receiveCallback2(void* arg, udp_pcb*, pbuf*, const ip_addr_t*, uint16_t){
    *static_cast<bool*>(arg) = true;
}

/**
 * Creating a connection and therefore binding some port needs the port on which
 * we want to receive. In this case "destPort", NOT "srcPort"
 */
TEST_F(LwIp, ConnectionContainsInputPort){
    bool called = false;
    const uint16_t srcPort = 7050;
    const uint16_t destPort = 7060;
    ip4_addr addr;
    LWIP_PORT_INIT_IPADDR(&addr); // self

    rtps::UdpDriver transport{receiveCallback2, &called};
    rtps::PacketInfo packet;
    packet.destAddr = addr;
    packet.buffer = rtps::PBufWrapper(10);

    // This block just creates the input port TODO
    packet.destPort = srcPort;
    packet.srcPort = destPort;
    transport.sendFunction(packet);


    packet.destPort = destPort;
    packet.srcPort = srcPort;

    transport.sendFunction(packet);
    sys_msleep(20);
    EXPECT_TRUE(called);

}
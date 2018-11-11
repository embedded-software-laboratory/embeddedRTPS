#include "gtest/gtest.h"

/*
 * This test file is used to get a better understanding of how LwIp works and defines behavior assumptions.
 * No claim to completeness.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/storages/PBufWrapper.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/rtps.h"
#include <lwip/udp.h>
#include <lwip/sys.h>
#include <lwipcfg.h>
#include <lwip/tcpip.h>
#include <cstring>

TEST(LwIp, pcbAllocation){
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

TEST(LwIp, pcbAllocationMultiThreaded){
    uint8_t first = 0;
    uint8_t second = 1;
    sys_thread_new("First", writer, &first, 10000, 3);
    sys_thread_new("Seconds", writer, &second, 10000, 3);
    while(!ready[0] || !ready[1]);
}


bool callbackFinished = false;
uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};

void receiveCallback(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, uint16_t port){
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

TEST(LwIP, SendSelfChainedPBufs){

    rtps::init();
    const uint16_t port = 7050;
    ip4_addr addr;
    LWIP_PORT_INIT_IPADDR(&addr); // self

    rtps::UdpDriver driver;
    driver.createUdpConnection(addr, port, receiveCallback);


    pbuf* first = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);
    pbuf* second = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);

    EXPECT_EQ(second->ref, 1);

    first->next = second;
    first->tot_len = first->tot_len + second->len;
    second->ref++;

    std::memcpy(first->payload, data0, 8);
    std::memcpy(second->payload, data1, 8);

    driver.sendPacket(addr, port, *first);
    while(!callbackFinished);
}

TEST(LwIP, CanCombineAndSplitChains){
    pbuf* first = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);
    pbuf* second = pbuf_alloc(PBUF_TRANSPORT, 9, PBUF_POOL);
    pbuf* third = pbuf_alloc(PBUF_TRANSPORT, 8, PBUF_POOL);
    pbuf* fourth = pbuf_alloc(PBUF_TRANSPORT, 9, PBUF_POOL);
}
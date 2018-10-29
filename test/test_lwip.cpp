#include "gtest/gtest.h"

/*
 * This test file is used to get a better understanding of how LwIp works and defines behavior assumptions.
 * No claim to completeness.
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "lwip/udp.h"
#include "rtps/communication/PBufWrapper.h"
#include "rtps/rtps.h"
#include "lwip/sys.h"

TEST(LwIp, pcbAllocation){
    // TODO rtps::init needs to be called. Currently omitted because it's done in another test. Needs a better method
    for(int i=1; i < 10000; i++){
        pbuf* buff = pbuf_alloc(PBUF_TRANSPORT, i, PBUF_POOL);
        pbuf_free(buff);
    }
}

bool ready[2] = {false, false};
void writer(void* args){
    for(int i=1; i < 10000; i++){
        LOCK_TCPIP_CORE();
        pbuf* buff = pbuf_alloc(PBUF_TRANSPORT, i, PBUF_POOL);
        UNLOCK_TCPIP_CORE();
        pbuf_free(buff);
    }
    ready[*((int*)args)] = true;
}

TEST(LwIp, pcbAllocationMultiThreaded){
    /*rtps::init();
    int first = 0;
    int second = 1;
    sys_thread_new("First", writer, &first, 10000, 3);
    sys_thread_new("Seconds", writer, &second, 10000, 3);
    while(!first || !second);*/
}
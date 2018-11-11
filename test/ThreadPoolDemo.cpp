/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"
#include "rtps/entities/StatelessWriter.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(){
    rtps::init();

    const uint16_t port = 7050;
    ip4_addr addr;
    IP4_ADDR((&addr), 192,168,0, 42);

    rtps::threadPool.addConnection(addr, port);
    bool started = rtps::threadPool.startThreads();
    if(!started){
        std::cout << "Failed starting threads";
        return 1;
    }

    rtps::StatelessWriter writer(rtps::TopicKind_t::NO_KEY);

    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};

    std::array<uint8_t*, 4> dataArray = {data0, data1, data2, data3};
    uint32_t i = 0;

    while(true){
        writer.newChange(rtps::ChangeKind_t::ALIVE, dataArray[i%4], 8);
        rtps::threadPool.addWorkload(static_cast<rtps::Writer&>(writer));
        i++;
        sys_msleep(20+(i%200));
    }

}
#pragma clang diagnostic pop
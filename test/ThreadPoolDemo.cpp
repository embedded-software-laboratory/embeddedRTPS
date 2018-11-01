/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(){
    rtps::init();

    const uint16_t port = 7050;
    ip4_addr addr;
    IP4_ADDR((&addr), 192,168,0, 42);

    ThreadPool pool;
    pool.addConnection(addr, port);
    bool started = pool.startThreads();
    if(!started){
        std::cout << "Failed starting threads";
        return 1;
    }

    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};
    std::array<Workload_t, 4> worklist = {{{addr, port, 8, data0}, {addr, port, 8, data1}, {addr, port, 8, data2}, {addr, port, 8, data3}}};

    uint32_t i = 0;

    while(true){
        Workload_t work = worklist[i%4]; // Copy
        //pool.addWorkload(std::move(work));
        i++;
        sys_msleep(20+(i%200));
    }

}
#pragma clang diagnostic pop
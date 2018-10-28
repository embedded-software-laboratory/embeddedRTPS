/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"

// TODO How to test this appropriately? Template? Dependency Injection?

int main(){
    rtps::init();

    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};

    const uint16_t port = 7050;
    ip4_addr addr;
    IP4_ADDR((&addr), 192,168,0, 100);

    std::array<Workload_t, 4> worklist = {{{addr, port, 8, data0}, {addr, port, 8, data1}, {addr, port, 8, data2}, {addr, port, 8, data3}}};
    ThreadPool pool;
    pool.addConnection(addr, port);
    bool started = pool.startThreads();
    if(!started){
        std::cout << "Failed starting threads";
        return 1;
    }
    int i = 0;

    while(true){
        pool.addWorkload(worklist[i%4]);
        i++;
        sys_msleep(20+(i%200));
    }
}
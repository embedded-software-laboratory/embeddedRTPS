/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"
#include "rtps/discovery/SPDP.h"
#include "rtps/entities/StatelessWriter.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(){
    rtps::init();

    rtps::ThreadPool pool;
    bool started = pool.startThreads();
    if(!started){
        std::cout << "Failed starting threads";
        return 1;
    }

    rtps::SPDPAgent parDiscovery{pool};
    parDiscovery.start();

    rtps::Locator_t locator = rtps::Locator_t::createUDPv4Locator(192, 168, 0, 248, 7050);
    rtps::StatelessWriter writer(rtps::TopicKind_t::NO_KEY, locator, &pool);

    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};

    std::array<uint8_t*, 4> dataArray = {data0, data1, data2, data3};
    uint32_t i = 0;

    while(true){

        auto change = writer.newChange(rtps::ChangeKind_t::ALIVE, dataArray[i%4], 8);
        i++;
        sys_msleep(20+(i%200));
        writer.removeChange(change);

    }

}


#pragma clang diagnostic pop
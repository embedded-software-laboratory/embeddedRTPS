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
    rtps::Domain domain;
    rtps::ThreadPool pool(domain);
    bool started = pool.startThreads();
    if(!started){
        std::cout << "Failed starting threads";
        return 1;
    }

    rtps::GuidPrefix_t guidPrefix{0,1,2,3,4,5,6,7,8,9,10,11};
    rtps::Participant part(guidPrefix, 1);
    rtps::SPDPAgent parDiscovery(pool, part);
    parDiscovery.start();

    /*
    rtps::Locator_t locator = rtps::Locator_t::createUDPv4Locator(192, 168, 0, 248, 7050);
    rtps::StatelessWriter writer(rtps::TopicKind_t::NO_KEY, locator, &pool, rtps::GUIDPREFIX_UNKNOWN, rtps::ENTITYID_UNKNOWN);

    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};

    std::array<uint8_t*, 4> dataArray = {data0, data1, data2, data3};
    uint32_t i = 0;
    */
    while(true){
        /*
        writer.newChange(rtps::ChangeKind_t::ALIVE, dataArray[i%4], 8);
        i++;
        sys_msleep(20+(i%200));
        */
    }

}


#pragma clang diagnostic pop
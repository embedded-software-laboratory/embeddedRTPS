/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"
#include "rtps/discovery/SPDP.h"
#include "rtps/entities/StatelessWriter.h"
#include "rtps/entities/Domain.h"


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(){
    rtps::init();
    rtps::Domain domain;
    domain.start();

    rtps::Participant* part = domain.createParticipant();
    if(part == nullptr){
        printf("Failed to create participant");
        return 1;
    }

    rtps::Locator_t locator = rtps::Locator_t::createUDPv4Locator(192, 168, 0, 248, 7050);
    rtps::Writer* writer = domain.createWriter(*part, locator, false);
    if(writer == nullptr){
        printf("Failed to create writer");
        return 2;
    }
    uint8_t data0[] = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    uint8_t data1[] = {'d', 'e', 'c', 'a', 'f', 'b', 'a', 'd'};
    uint8_t data2[] = {'b', 'a', 'd', 'c', 'o', 'd', 'e', 'd'};
    uint8_t data3[] = {'b', 'a', 'd', 'c', 'a', 'b', '1', 'e'};

    std::array<uint8_t*, 4> dataArray = {data0, data1, data2, data3};
    uint32_t i = 0;
    while(true){

        writer->newChange(rtps::ChangeKind_t::ALIVE, dataArray[i%4], 8);
        i++;
        sys_msleep(20+(i%200));

    }

}


#pragma clang diagnostic pop
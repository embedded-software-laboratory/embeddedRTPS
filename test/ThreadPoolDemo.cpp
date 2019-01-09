/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/ThreadPool.h"
#include "rtps/rtps.h"
#include "rtps/discovery/SPDPAgent.h"
#include "rtps/entities/Writer.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/Domain.h"

#define PUB 0

void receiveCallback(void* /*callee*/, rtps::ReaderCacheChange& /*cacheChange*/){
    printf("Received hello world message.\n");
}


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(){
    rtps::init();
    rtps::Domain domain;
    domain.start();

    auto part = domain.createParticipant();
    //rtps::Participant* part2 = domain.createParticipant();

    if(part == nullptr){
        printf("Failed to create participant");
        return 1;
    }

    char topicName[] = "HelloWorldTopic";
    char typeName[] = "HelloWorld";

#if PUB
    rtps::Writer* writer = domain.createWriter(*part, topicName, typeName, true);
    if(writer == nullptr){
        printf("Failed to create writer");
        return 2;
    }

    char message[] = "Hello World";
    uint8_t buffer[16];
    ucdrBuffer microbuffer;
    uint32_t i = 0;
    while(true){
        ucdr_init_buffer(&microbuffer, buffer, sizeof(buffer)/ sizeof(buffer[0]));
        ucdr_serialize_uint32_t(&microbuffer, i);
        ucdr_serialize_array_char(&microbuffer, message, sizeof(message)/sizeof(message[0]));
        writer->newChange(rtps::ChangeKind_t::ALIVE, buffer, sizeof(buffer)/ sizeof(buffer[0]));
        sys_msleep(5000);

    }

#else
    rtps::Reader* reader = domain.createReader(*part, topicName, typeName, true);
    reader->registerCallback(receiveCallback, nullptr);

    while(true){
        // Nothing to do. Just wait...
    }

#endif

}

#pragma clang diagnostic pop
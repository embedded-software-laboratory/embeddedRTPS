/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/rtps.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Domain.h"

#define PUB 0

void receiveCallback(void* /*callee*/, rtps::ReaderCacheChange& cacheChange){
    uint8_t buffer[50];
    bool success = cacheChange.copyInto(buffer, 50);
    if(success){
        uint8_t offset = 4; // Encoding info and options
        printf("Received hello world message with index: %u\n", *reinterpret_cast<uint32_t*>(buffer + offset));
    }else{
        printf("Received hello world message but copying failed\n");
    }

}

void startProgram(void*);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main() {
    rtps::init();

    sys_thread_new("Main Program", startProgram, nullptr, 1024, 3);
    while(true);
}

void startProgram(void* /*args*/){
    rtps::Domain domain;
    domain.start();

    auto part = domain.createParticipant();

    if(part == nullptr){
        printf("Failed to create participant");
        return;
    }

    char topicName[] = "HelloWorldTopic";
    char typeName[] = "HelloWorld";

    //auto part2 = domain.createParticipant();
    //rtps::Reader* reader = domain.createReader(*part2, topicName, typeName, true);
    //reader->registerCallback(receiveCallback, nullptr);

#if PUB
    rtps::Writer* writer = domain.createWriter(*part, topicName, typeName, true);
    if(writer == nullptr){
        printf("Failed to create writer");
        return;
    }

    char message[] = "Hello World";
    uint8_t buffer[20];
    ucdrBuffer microbuffer;
    uint32_t i = 0;
    while(true){
        ucdr_init_buffer(&microbuffer, buffer, sizeof(buffer)/ sizeof(buffer[0]));
        ucdr_serialize_array_uint8_t(&microbuffer, rtps::SMElement::SCHEME_CDR_LE.data(), rtps::SMElement::SCHEME_CDR_LE.size());
        ucdr_serialize_uint16_t(&microbuffer, 0); // options
        ucdr_serialize_uint32_t(&microbuffer, i++);
        ucdr_serialize_array_char(&microbuffer, message, sizeof(message)/sizeof(message[0]));
        auto change = writer->newChange(rtps::ChangeKind_t::ALIVE, buffer, sizeof(buffer)/ sizeof(buffer[0]));
        if(change == nullptr){
            printf("History full.\n");
            while(true);
        }
        //sys_msleep(5000);

    }

#else

    rtps::Reader* reader = domain.createReader(*part, topicName, typeName, false);
    if(reader == nullptr){
        printf("Failed to create reader\n");
        return;
    }

    reader->registerCallback(receiveCallback, nullptr);

    while(true){
        // Nothing to do. Just wait...
    }

#endif
}

#pragma clang diagnostic pop
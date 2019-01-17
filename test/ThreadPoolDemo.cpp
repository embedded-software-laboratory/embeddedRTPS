/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <iostream>
#include "rtps/rtps.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Domain.h"

#include <vector>
#include <chrono>

#define MEASUREMENT 1

#if MEASUREMENT
#define RESPONDER 1
#else
#define PUB 0
#endif

std::array<uint8_t,50> buffer;
bool send = true;
std::vector<std::chrono::duration<double, std::micro>> times;
std::chrono::steady_clock::time_point start;
std::chrono::duration<double, std::micro> overhead;

void responderCallback(void* vp_writer, rtps::ReaderCacheChange& cacheChange){
    auto writer = static_cast<rtps::Writer*>(vp_writer);

    bool success = cacheChange.copyInto(buffer.data(), buffer.size());
    if(success){
        writer->newChange(rtps::ChangeKind_t::ALIVE, buffer.data(), buffer.size());
    }else{
        printf("Received hello world message but copying failed\n");
    }
}

void measurementCallback(void* /*callee*/, rtps::ReaderCacheChange& cacheChange){
    auto end = std::chrono::steady_clock::now();
    times.push_back(std::chrono::duration<double, std::micro>(end - start) - overhead);
    send = true;
}

void receiveCallback(void* vp_writer, rtps::ReaderCacheChange& cacheChange){
    auto writer = static_cast<rtps::Writer*>(vp_writer);

    bool success = cacheChange.copyInto(buffer.data(), buffer.size());
    if(success){
        uint8_t offset = 4; // Encoding info and options
        printf("Received hello world message with index: %u\n", *reinterpret_cast<uint32_t*>(buffer.data() + offset));
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
    std::cout << "Size of domain: " << sizeof(domain) << "bytes.\n";
    domain.start();

    auto part = domain.createParticipant();

    if(part == nullptr){
        printf("Failed to create participant");
        return;
    }

    char topicName[] = "HelloWorldTopic";
    char typeName[] = "HelloWorld";

#if MEASUREMENT

    #if RESPONDER
        rtps::Writer* writer = domain.createWriter(*part, topicName, typeName, false);
        rtps::Reader* reader = domain.createReader(*part, topicName, typeName, false);
        if(writer == nullptr || reader == nullptr){
            printf("Failed to create endpoints.\n");
            return;
        }
        reader->registerCallback(responderCallback, writer);
    #else

        // Calculate overhead
        std::chrono::steady_clock::time_point start_oh;
        std::chrono::steady_clock::time_point end_oh;
        start_oh = std::chrono::steady_clock::now();
        for(int i=0; i < 1000; ++i){
            end_oh = std::chrono::steady_clock::now();
        }
        overhead = std::chrono::duration<double, std::micro>(end_oh - start_oh) / 1001;
        std::cout << "Chrono Overhead " << overhead.count() << " ns" << std::endl;


        // Start
        rtps::Writer* writer = domain.createWriter(*part, topicName, typeName, false);
        rtps::Reader* reader = domain.createReader(*part, topicName, typeName, false);
        if(writer == nullptr || reader == nullptr){
            printf("Failed to create endpoints.\n");
            return;
        }

        reader->registerCallback(measurementCallback, nullptr);

        char message[] = "Hello World";
        uint8_t buffer[20];
        ucdrBuffer microbuffer;

        std::cout << "Waiting 15 sec for startup...." << '\n';
        sys_msleep(15000); // Wait for initialization
        std::cout << "Go!" << '\n';
        uint32_t i = 0;
        while(true){
            ucdr_init_buffer(&microbuffer, buffer, sizeof(buffer)/ sizeof(buffer[0]));
            ucdr_serialize_array_uint8_t(&microbuffer, rtps::SMElement::SCHEME_CDR_LE.data(), rtps::SMElement::SCHEME_CDR_LE.size());
            ucdr_serialize_uint16_t(&microbuffer, 0); // options
            ucdr_serialize_uint32_t(&microbuffer, i++);
            ucdr_serialize_array_char(&microbuffer, message, sizeof(message)/sizeof(message[0]));

            start = std::chrono::steady_clock::now();
            auto change = writer->newChange(rtps::ChangeKind_t::ALIVE, buffer, sizeof(buffer)/ sizeof(buffer[0]));
            if(change == nullptr){
                printf("History full.\n");
                while(true);
            }else{
                std::cout << "Added change with id " <<  i << '\n';
            }
            send = false;
            while(!send);

        }
    #endif

#else

    #if PUB
        rtps::Writer* writer = domain.createWriter(*part, topicName, typeName, false);
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
            }else{
                printf("Added change with id %i\n", i);
            }
            sys_msleep(200);

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

#endif

        while(true);
}

#undef MEASUREMENT
#undef RESPONDER
#undef PUB

#pragma clang diagnostic pop
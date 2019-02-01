/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/rtps.h"
#include "rtps/messages/MessageTypes.h"
#include "rtps/entities/Domain.h"

#include <iostream>


bool ok = false;

void receiveCallback(void* /*callee*/, rtps::ReaderCacheChange& cacheChange){
    std::array<uint8_t, 25> buffer;
    bool success = cacheChange.copyInto(buffer.data(), buffer.size());
    if(success){
        printf("%s", buffer.data());
    }else{
        printf("Received message but copying failed\n");
    }
    ok = true;
}


void startProgram(void*);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main() {
    rtps::init();

    rtps::Domain domain;
    std::cout << "Size of domain: " << sizeof(domain) << "bytes.\n";
    domain.start();

    auto part = domain.createParticipant();

    if(part == nullptr){
        printf("Failed to create participant");
        return - 1;
    }

    char topicNamePub2Sub[] = "TFT";
    char typeNamePub2Sub[] = "TFTMessage";

    char topicNameSub2Pub[] = "Answer";
    char typeNameSub2Pub[] = "AnswerType";

        rtps::Writer *writer = domain.createWriter(*part, topicNamePub2Sub, typeNamePub2Sub, false);
        rtps::Reader* reader = domain.createReader(*part, topicNameSub2Pub, typeNameSub2Pub, false);
        if (writer == nullptr || reader == nullptr) {
            printf("Failed to create endpoints\n");
            return - 1;
        }
        reader->registerCallback(receiveCallback, nullptr);

        char buffer[51];
        uint8_t line;
        while (true) {
            ok = false;
            std::cout << "Enter line: ";
            std::cin >> line;
            buffer[0] = line;
            std::cout << "\nEnter text:";
            std::cin.get(buffer + 1, 50);
            writer->newChange(rtps::ChangeKind_t::ALIVE, reinterpret_cast<uint8_t*>(buffer), sizeof(buffer)/ sizeof(buffer[0]));
            while(!ok);
        }
}

#undef MEASUREMENT
#undef RESPONDER
#undef PUB

#pragma clang diagnostic pop
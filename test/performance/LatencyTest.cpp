/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <array>
#include <vector>
#include <chrono>
#include <iostream>


#include "rtps/rtps.h"
#include "rtps/entities/Domain.h"
#include "test/performance/MeasurementUnit.h"
#include "test/performance/ResponderUnit.h"

#include <algorithm>

static bool isResponder = false;
static const uint32_t numSamples = 10000;
static const uint32_t messagesSizesInBytes[] = {12, 28, 60, 124, 252, 508, 1020, 2044, 4092, 8188, 16380};

void startProgram(void*);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(int argc,char* argv[]) {
    if(argc != 2){ // main + 1 argument
        std::cout << std::string(R"("Please execute with argument "measure" or "respond")") << std::endl;
        return -1;
    }

    if(strcmp(argv[1], "measure") == 0){
        std::cout << "Starting measurement unit\n";
        isResponder = false;
    }else if(strcmp(argv[1], "respond") == 0){
        std::cout << "Starting responder unit\n";
        isResponder = true;
    }else{
        std::cout << std::string(R"("Please execute with argument "measure" or "respond")") << std::endl;
        return -1;
    }

    rtps::init();

    sys_thread_new("Main Program", startProgram, nullptr, 1024, 3);
    while(true);
}

void startProgram(void* /*args*/){

    if(isResponder) {
        uint32_t maxSize = *std::max_element(std::begin(messagesSizesInBytes), std::end(messagesSizesInBytes));
        rtps::tests::ResponderUnit responderUnit(maxSize);
        responderUnit.run();
    }else{
        std::vector<uint32_t> tmpVector(messagesSizesInBytes, messagesSizesInBytes + sizeof(messagesSizesInBytes)/sizeof(messagesSizesInBytes[0]));
        rtps::tests::MeasurementUnit measurementUnit(numSamples, std::move(tmpVector));
        measurementUnit.run();
    }
}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MEASUREMENTUNIT_H
#define RTPS_MEASUREMENTUNIT_H

#include <rtps/entities/Domain.h>

#include "rtps/config.h"

#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace rtps{
    namespace tests{

        class MeasurementUnit{
        public:
            MeasurementUnit(uint32_t numSamples, std::vector<uint32_t>&& messageSizesInBytes);
            void run();

        private:
            Domain m_domain;
            Writer* mp_dataWriter;
            Reader* mp_dataReader;
            std::mutex m_mutex;
            std::condition_variable m_condVar;
            bool m_receivedResponse;


            uint32_t m_numSamples;
            uint32_t m_numBytes;
            std::vector<uint32_t> m_messageSizesInBytes;

            std::chrono::duration<double, std::micro> m_overhead;

            std::vector<std::chrono::duration<double, std::micro>> m_times;
            std::chrono::steady_clock::time_point m_start;

            void prepareRTPS();
            void calculateTimerOverhead();

            static void measurementJumppad(void* callee, rtps::ReaderCacheChange& cacheChange);
            void measurementCallback(rtps::ReaderCacheChange& cacheChange);
            void prepare(uint32_t numBytes);
            void runWithSpecificSize();
            void evaluate();
        };
    }
}

#endif //RTPS_MEASUREMENTUNIT_H

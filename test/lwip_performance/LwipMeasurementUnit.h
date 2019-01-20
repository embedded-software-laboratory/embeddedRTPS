/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_MEASUREMENTUNIT_H
#define RTPS_MEASUREMENTUNIT_H

#include <rtps/utils/udpUtils.h>
#include <lwip/udp.h>

#include "rtps/config.h"

#include <chrono>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace rtps{
    namespace tests{

        class LwipMeasurementUnit{
        public:
            LwipMeasurementUnit(uint32_t numSamples, std::vector<uint32_t>&& messageSizesInBytes);
            void run();

        private:
            udp_pcb* m_pcb;
            uint16_t m_recvPort = 7410;
            uint16_t m_destPort = 7410;
            ip4_addr m_destAddr = transformIP4ToU32(192, 168, 0, 42);

            std::mutex m_mutex;
            std::condition_variable m_condVar;
            bool m_receivedResponse;


            uint32_t m_numSamples;
            uint32_t m_numBytes;
            std::vector<uint32_t> m_messageSizesInBytes;

            std::chrono::duration<double, std::micro> m_overhead;

            std::vector<std::chrono::duration<double, std::micro>> m_times;
            std::chrono::steady_clock::time_point m_start;

            void prepareLWIP();
            void calculateTimerOverhead();
            static void measurementJumppad(void* callee, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port);
            void measurementCallback(pbuf* pbuf);
            void prepare(uint32_t numBytes);
            void runWithSpecificSize();
            void evaluate();
        };
    }
}

#endif //RTPS_MEASUREMENTUNIT_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "test/lwip_performance/LwipMeasurementUnit.h"

#include "rtps/rtps.h"
#include "rtps/entities/Domain.h"
#include "test/lwip_performance/LwipLatencyTopic.h"

#include <iostream>
#include <numeric>
#include <cmath>
#include <algorithm>


using rtps::tests::LwipMeasurementUnit;

LwipMeasurementUnit::LwipMeasurementUnit(uint32_t numSamples, std::vector<uint32_t>&& messageSizesInBytes)
    : m_receivedResponse(false), m_numSamples(numSamples), m_messageSizesInBytes(messageSizesInBytes){
    prepareLWIP();
    calculateTimerOverhead();
}

void LwipMeasurementUnit::prepareLWIP(){
    LOCK_TCPIP_CORE();
    m_pcb = udp_new();
    err_t err = udp_bind(m_pcb, IP_ADDR_ANY, m_recvPort); //to receive multicast

    if(err != ERR_OK && err != ERR_USE){
        printf("Failed to bind to port%u: error %u\n", m_recvPort, err);
        return;
    }

    udp_recv(m_pcb, LwipMeasurementUnit::measurementJumppad, this);
    printf("Successfully created UDP connection on port %u \n", m_recvPort);
    UNLOCK_TCPIP_CORE();
}

void LwipMeasurementUnit::calculateTimerOverhead(){
    std::chrono::steady_clock::time_point startOverhead;
    std::chrono::steady_clock::time_point endOverhead;

    startOverhead = std::chrono::steady_clock::now();
    for(int i=0; i < 1000; ++i){
        endOverhead = std::chrono::steady_clock::now();
    }
    m_overhead = std::chrono::duration<double, std::micro>(endOverhead - startOverhead) / 1001;
    std::cout << "Timer overhead: " << m_overhead.count() << " ns" << std::endl;

}

void LwipMeasurementUnit::measurementJumppad(void* callee, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port){
    auto measurementUnit = static_cast<LwipMeasurementUnit*>(callee);
    measurementUnit->measurementCallback(pbuf);
}

void LwipMeasurementUnit::measurementCallback(pbuf* pbuf){
    const auto end = std::chrono::steady_clock::now();
    m_times.push_back(std::chrono::duration<double, std::micro>(end - m_start) - m_overhead);

    //std::cout << "Duration: " << m_times[m_times.size()-1].count() << "ns" << '\n';
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if(m_receivedResponse){
            std::cout << "Received response was already signalized\n";
        }
        m_receivedResponse = true;

        pbuf_free(pbuf);
    }
    m_condVar.notify_one();
}

void LwipMeasurementUnit::run() {

    printf("Printing round-trip times in us, statistics for %d samples\n", m_numSamples);
    printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
    printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");

    for (auto numBytes : m_messageSizesInBytes) {
        prepare(numBytes);
        runWithSpecificSize();
        evaluate();
    }
}

void LwipMeasurementUnit::prepare(uint32_t numBytes){
    m_numBytes = numBytes;
    m_times.clear();
    m_times.reserve(numBytes);
    // TODO Do we need to clear the history?
}

void LwipMeasurementUnit::runWithSpecificSize(){
    LatencyPacket packet(m_numBytes);


    for(uint64_t i=0; i < m_numSamples; i++) {
        static_assert(sizeof(m_numSamples) <= sizeof(i), "MeasurementUnit: loop variable too small.");

        pbuf* buffer = pbuf_alloc(PBUF_TRANSPORT, m_numBytes, PBUF_POOL);
        sys_msleep(10);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_receivedResponse = false;
        m_start = std::chrono::steady_clock::now();
        LOCK_TCPIP_CORE();
        err_t err = udp_sendto(m_pcb, buffer, &m_destAddr, m_destPort);
        UNLOCK_TCPIP_CORE();
        if(err != ERR_OK){
            printf("UDP TRANSMIT NOT SUCCESSFUL %s:%u size: %u err: %i\n", ipaddr_ntoa(&m_destAddr), m_destPort, buffer->tot_len, err);
            continue;
        }

        //m_condVar.wait(lock, [this]{return this->m_receivedResponse;});
        m_condVar.wait_for(lock, std::chrono::duration<double, std::milli>(5000),
                           [this]{return this->m_receivedResponse;});
        m_receivedResponse = true;
        pbuf_free(buffer);
    }
}

void LwipMeasurementUnit::evaluate(){

    const auto minDuration = *std::min_element(m_times.begin(), m_times.end());
    const auto maxDuration = *std::max_element(m_times.begin(), m_times.end());
    const auto meanDuration = std::accumulate(m_times.begin(), m_times.end(), std::chrono::duration<double,std::micro>(0)).count()/m_times.size();

    double variance = 0;
    for(const auto& duration : m_times){
        variance += pow(duration.count() - meanDuration, 2);
    }

    const double stddev = sqrt(variance)/m_times.size();

    std::sort(m_times.begin(), m_times.end());

    const std::array<double, 4> quantiles = {0.5, 0.9, 0.99, 0.9999};
    std::vector<double> quantilesResult;
    for(auto q : quantiles){
        auto elemIdx = static_cast<size_t>(std::ceil(m_times.size()*q) - 1);
        if(elemIdx < m_times.size()){
            quantilesResult.push_back(m_times[elemIdx].count());
        }else{
            quantilesResult.push_back(NAN);
        }

    }

    static_assert(quantiles.size() == 4, "Not enough quantiles.");
    printf("%8u,%8" PRIu64 ",%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
           m_numBytes, m_times.size(), stddev, meanDuration, minDuration.count(),
           quantilesResult[0], quantilesResult[1], quantilesResult[2], quantilesResult[3],
           maxDuration.count());
}

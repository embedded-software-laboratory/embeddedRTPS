/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "test/performance/MeasurementUnit.h"

#include "rtps/rtps.h"
#include "rtps/entities/Domain.h"
#include "test/performance/LatencyTopic.h"

#include <iostream>
#include <numeric>
#include <cmath>
#include <algorithm>


using rtps::tests::MeasurementUnit;

MeasurementUnit::MeasurementUnit(uint32_t numSamples, std::vector<uint32_t>&& messageSizesInBytes)
    : m_receivedResponse(false), m_numSamples(numSamples), m_messageSizesInBytes(messageSizesInBytes){
    prepareRTPS();
    calculateTimerOverhead();
}

void MeasurementUnit::prepareRTPS(){
    m_domain.start();
    auto part = m_domain.createParticipant();
    if(part == nullptr){
       std::cout << "Failed to create participant\n";
        return;
    }
    mp_dataWriter = m_domain.createWriter(*part, LatencyPacket::topicName, LatencyPacket::typeName, false);
    mp_dataReader = m_domain.createReader(*part, LatencyPacket::topicName, LatencyPacket::typeName, false);


    if(mp_dataWriter == nullptr || mp_dataReader == nullptr){
        std::cout << "Failed to create endpoints.\n";
        return;
    }

    mp_dataReader->registerCallback(measurementJumppad, this);
}

void MeasurementUnit::calculateTimerOverhead(){
    std::chrono::steady_clock::time_point startOverhead;
    std::chrono::steady_clock::time_point endOverhead;

    startOverhead = std::chrono::steady_clock::now();
    for(int i=0; i < 1000; ++i){
        endOverhead = std::chrono::steady_clock::now();
    }
    m_overhead = std::chrono::duration<double, std::micro>(endOverhead - startOverhead) / 1001;
    std::cout << "Timer overhead: " << m_overhead.count() << " ns" << std::endl;

}

void MeasurementUnit::measurementJumppad(void* callee, rtps::ReaderCacheChange& cacheChange){
    auto measurementUnit = static_cast<MeasurementUnit*>(callee);
    measurementUnit->measurementCallback(cacheChange);
}

void MeasurementUnit::measurementCallback(rtps::ReaderCacheChange& /*cacheChange*/){
    const auto end = std::chrono::steady_clock::now();
    m_times.push_back(std::chrono::duration<double, std::micro>(end - m_start) - m_overhead);

    //std::cout << "Duration: " << m_times[m_times.size()-1].count() << "ns" << '\n';
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if(m_receivedResponse){
            std::cout << "Received response was already signalized\n";
        }
        m_receivedResponse = true;
    }
    m_condVar.notify_one();
}

void MeasurementUnit::run() {
    std::cout << "Waiting 15 sec for startup...." << '\n';
    sys_msleep(15000); // Wait for initialization
    std::cout << "Go!" << '\n';

    printf("Printing round-trip times in us, statistics for %d samples\n", m_numSamples);
    printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
    printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");

    for (auto numBytes : m_messageSizesInBytes) {
        prepare(numBytes);
        runWithSpecificSize();
        evaluate();
    }
}

void MeasurementUnit::prepare(uint32_t numBytes){
    m_numBytes = numBytes;
    m_times.clear();
    m_times.reserve(numBytes);
    // TODO Do we need to clear the history?
}

void MeasurementUnit::runWithSpecificSize(){
    LatencyPacket packet(m_numBytes);

    for(uint64_t i=0; i < m_numSamples; i++) {
        static_assert(sizeof(m_numSamples) <= sizeof(i), "MeasurementUnit: loop variable too small.");

        std::unique_lock<std::mutex> lock(m_mutex);
        m_receivedResponse = false;

        m_start = std::chrono::steady_clock::now();
        auto change = mp_dataWriter->newChange(rtps::ChangeKind_t::ALIVE, packet.data.data(), packet.data.size());
        if(change == nullptr){
            std::cout << "History full. Abort. \n";
            return;
        }
        //m_condVar.wait(lock, [this]{return this->m_receivedResponse;});
        m_condVar.wait_for(lock, std::chrono::duration<double, std::milli>(2000),
                           [this]{return this->m_receivedResponse;});
    }
}

void MeasurementUnit::evaluate(){

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
    printf("%8u,%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
           m_numBytes, m_times.size(), stddev, meanDuration, minDuration.count(),
           quantilesResult[0], quantilesResult[1], quantilesResult[2], quantilesResult[3],
           maxDuration.count());
}

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "config.h"
#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/ThreadSafeCircularBuffer.h"

#include <array>

namespace rtps {

    class Domain;

    class ThreadPool {
    public:

        explicit ThreadPool(Domain& domain);
        struct Workload_t{
            Writer* pWriter;
            uint8_t numCacheChangesToSend;
        };

        bool startThreads();

        void stopThreads();

        void clearQueues();

        bool addConnection(const ip4_addr_t& addr, const ip4_port_t port);

        void addWorkload(Workload_t workload);

    private:
        Domain& domain;
        bool running = false;
        UdpDriver transport;
        std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> writers;

        ThreadSafeCircularBuffer<Workload_t, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> inputQueue;
        ThreadSafeCircularBuffer<PBufWrapper, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> outputQueue;

        static void readCallback(void* arg, udp_pcb* pcb, pbuf* p, const ip_addr_t* addr, ip4_port_t port);

        static void sendFunction(void* arg);

        static void writerFunction(void* arg);

    };
}

#endif //RTPS_THREADPOOL_H

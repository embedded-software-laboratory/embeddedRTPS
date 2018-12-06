/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "config.h"
#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/ThreadSafeCircularBuffer.h"

#include <array>

namespace rtps {

    class Domain;
    class Writer;

    class ThreadPool {
    public:

        explicit ThreadPool(Domain& domain);

        ~ThreadPool();

        struct Workload_t{
            Writer* pWriter;
            uint8_t numCacheChangesToSend;
        };

        struct PacketInfo{
            Ip4Port_t srcPort;
            ip4_addr_t destAddr;
            Ip4Port_t destPort;
            PBufWrapper buffer;
        };

        bool startThreads();

        void stopThreads();

        void clearQueues();

        void addWorkload(Workload_t workload);

    private:
        Domain& domain;
        bool running = false;
        UdpDriver transport;
        std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> writers;

        ThreadSafeCircularBuffer<Workload_t, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> inputQueue;
        ThreadSafeCircularBuffer<PacketInfo, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> outputQueue;

        static void readCallback(void* arg, udp_pcb* pcb, pbuf* p, const ip_addr_t* addr, Ip4Port_t port);

        static void sendFunction(void* arg);

        static void writerFunction(void* arg);

    };
}

#endif //RTPS_THREADPOOL_H

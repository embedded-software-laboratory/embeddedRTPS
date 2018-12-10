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
            Workload_t() : p_writer(nullptr){};
            explicit Workload_t(Writer* writer) : p_writer(writer){};

            Writer* p_writer;
        };

        bool startThreads();
        void stopThreads();

        void clearQueues();
        void addWorkload(Workload_t workload);

        static void readCallback(void* arg, udp_pcb* pcb, pbuf* p, const ip_addr_t* addr, Ip4Port_t port);


    private:
        Domain& domain;
        bool running = false;
        std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> writers;

        ThreadSafeCircularBuffer<Workload_t, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> inputQueue;
        ThreadSafeCircularBuffer<UdpDriver::PacketInfo, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> outputQueue;

        static void writerFunction(void* arg);

    };
}

#endif //RTPS_THREADPOOL_H

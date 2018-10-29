/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "config.h"
#include "lwip/sys.h"
#include "rtps/communication/PBufWrapper.h"
#include "rtps/utils/ThreadSafeCircularBuffer.h"
#include "rtps/communication/UdpDriver.h"

#include <array>

struct Workload_t{
    ip4_addr_t addr;
    uint16_t port;
    uint16_t size;
    uint8_t* data;
};

class ThreadPool{
private:
    bool running = false;
    UdpDriver transport;
    std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> writers;

    ThreadSafeCircularBuffer<Workload_t, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> inputQueue;
    ThreadSafeCircularBuffer<PBufWrapper, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> outputQueue;

    static void readCallback(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, ip4_port_t port);
    static void sendFunction(void *arg);
    static void writerFunction(void *arg);

public:
    bool startThreads();
    void stopThreads();
    bool addConnection(const ip4_addr_t &addr, const ip4_port_t port);
    void addWorkload(Workload_t&& work);


};

#endif //RTPS_THREADPOOL_H

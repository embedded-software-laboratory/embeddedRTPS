/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "lwip/sys.h"
#include "UdpDriver.h"
#include "config.h"
#include <array>

struct Workload_t{
    ip4_addr_t addr;
    uint16_t port;
    uint16_t size;
    uint8_t* data;
};

class ThreadPool{
public:
    bool startThreads();
    void stopThreads();
    bool addConnection(ip4_addr_t& addr, ip4_port_t port);
    void addWorkload(Workload_t work);

private:
    bool running;
    UdpDriver transportDriver;
    std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> writers;

    std::array<Workload_t, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> workloadQueue;
    sys_mutex_t queueMutex;
    static_assert(Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH < UINT32_MAX);
    uint32_t head = 0;
    uint32_t tail = 0;

    static void writerFunction(void *arg);
    static void callback(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, ip4_port_t port);
};

#endif //RTPS_THREADPOOL_H

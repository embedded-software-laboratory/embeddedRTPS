/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "config.h"
#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/ThreadSafeCircularBuffer.h"

#include <array>

namespace rtps {

    class Writer;


    class ThreadPool {
    public:
        typedef void(*receiveJumppad_fp)(void* callee, const PacketInfo& packet);

        ThreadPool(receiveJumppad_fp receiveCallback, void* callee);

        ~ThreadPool();

        bool startThreads();
        void stopThreads();

        void clearQueues();
        bool addWorkload(Writer* workload);
        bool addNewPacket(PacketInfo&& packet);

        static void readCallback(void* arg, udp_pcb* pcb, pbuf* p, const ip_addr_t* addr, Ip4Port_t port);


    private:
        receiveJumppad_fp m_receiveJumppad;
        void* m_callee;
        bool m_running = false;
        std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> m_writers;
        std::array<sys_thread_t, Config::THREAD_POOL_NUM_READERS> m_readers;

        sys_sem_t m_readerNotificationSem;
        sys_sem_t m_writerNotificationSem;

        ThreadSafeCircularBuffer<Writer*, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> m_queueOutgoing;
        ThreadSafeCircularBuffer<PacketInfo, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH> m_queueIncoming;

        static void writerThreadFunction(void* arg);
        static void readerThreadFunction(void* arg);
        void doWriterWork();
        void doReaderWork();

    };
}

#endif //RTPS_THREADPOOL_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/ThreadPool.h"

#include "lwip/tcpip.h"
#include "rtps/entities/Writer.h"
#include "rtps/utils/udpUtils.h"



using rtps::ThreadPool;

#define THREAD_POOL_VERBOSE 0


ThreadPool::ThreadPool(receiveJumppad_fp receiveCallback, void* callee)
    : m_receiveJumppad(receiveCallback), m_callee(callee){

    if(!m_inputQueue.init() || !m_outputQueue.init()){
        return;
    }
    err_t inputErr = sys_sem_new(&m_readerNotificationSem, 0);
    err_t outputErr = sys_sem_new(&m_writerNotificationSem, 0);
#if THREAD_POOL_VERBOSE
    if(inputErr != ERR_OK || outputErr != ERR_OK){
        printf("ThreadPool: Failed to create Semaphores.\n");
    }
#endif
}

ThreadPool::~ThreadPool(){
    if(m_running){
        stopThreads();
        sys_msleep(500); // Quick fix for tests. The dtor should never be called in real application.
    }

    if(sys_sem_valid(&m_readerNotificationSem)){
        sys_sem_free(&m_readerNotificationSem);
    }
    if(sys_sem_valid(&m_writerNotificationSem)){
        sys_sem_free(&m_writerNotificationSem);
    }
}

bool ThreadPool::startThreads(){
    if(m_running){
        return true;
    }
    if(!sys_sem_valid(&m_readerNotificationSem) || !sys_sem_valid(&m_writerNotificationSem)){
        return false;
    }

    m_running = true;
    for(auto &thread : m_writers){
        // TODO ID, err check, waitOnStop
        thread = sys_thread_new("WriterThread", writerThreadFunction, this, Config::THREAD_POOL_WRITER_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
    }

    for(auto &thread : m_readers){
        // TODO ID, err check, waitOnStop
        thread = sys_thread_new("ReaderThread", readerThreadFunction, this, Config::THREAD_POOL_READER_STACKSIZE, Config::THREAD_POOL_READER_PRIO);
    }
    return true;
}

void ThreadPool::stopThreads() {
    m_running = false;
    // TODO make sure they have finished. Seems sufficient to be sufficient for tests.
    // No sufficient if threads shall actually be stopped during runtime.
    sys_msleep(500);
}

void ThreadPool::clearQueues(){
    m_inputQueue.clear();
    m_outputQueue.clear();
}

void ThreadPool::addWorkload(Workload_t workload){
    m_inputQueue.moveElementIntoBuffer(std::move(workload));
    sys_sem_signal(&m_writerNotificationSem);
}

bool ThreadPool::addNewPacket(PacketInfo&& packet){
    bool res = m_outputQueue.moveElementIntoBuffer(std::move(packet));
    if(res){
        sys_sem_signal(&m_readerNotificationSem);
    }
    return res;
}

void ThreadPool::writerThreadFunction(void* arg){
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
#if THREAD_POOL_VERBOSE
        printf("nullptr passed to writer function\n");
#endif
        return;
    }

    pool->doWriterWork();
}

void ThreadPool::doWriterWork(){
    while(m_running){
        Workload_t workload;
        auto isWorkToDo = m_inputQueue.moveFirstInto(workload);
        if(!isWorkToDo){
            sys_sem_wait(&m_writerNotificationSem);
            continue;
        }

        workload.p_writer->progress();
    }
}

void ThreadPool::readCallback(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* /*addr*/, Ip4Port_t port) {
    auto& pool = *static_cast<ThreadPool*>(args);

    PacketInfo packet;
    packet.destAddr = {0}; // not relevant
    packet.destPort = target->local_port;
    packet.srcPort = port;
    packet.buffer = PBufWrapper{pbuf};
    if(!pool.addNewPacket(std::move(packet))){
#if THREAD_POOL_VERBOSE
        printf("ThreadPool: dropped packet\n");
#endif
    }
}

void ThreadPool::readerThreadFunction(void* arg){
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
#if THREAD_POOL_VERBOSE
        printf("nullptr passed to reader function\n");
#endif
        return;
    }
        pool->doReaderWork();
}

void ThreadPool::doReaderWork(){

    while(m_running){
        PacketInfo packet;
        auto isWorkToDo = m_outputQueue.moveFirstInto(packet);
        if(!isWorkToDo) {
            sys_sem_wait(&m_readerNotificationSem);
            continue;
        }

        m_receiveJumppad(m_callee, const_cast<const PacketInfo&>(packet));
    }
}

#undef THREAD_POOL_VERBOSE

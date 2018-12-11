/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/ThreadPool.h"

#include "lwip/tcpip.h"
#include "rtps/entities/Domain.h"
#include "rtps/entities/Writer.h"
#include "rtps/utils/udpUtils.h"

using rtps::ThreadPool;


ThreadPool::ThreadPool(Domain& domain) : domain(domain){

}

ThreadPool::~ThreadPool(){
    stopThreads();
    sys_msleep(500); // Doesn't matter for real application. The dtor should never be called.
}

bool ThreadPool::startThreads(){
    if(running){
        return true;
    }
    if(!inputQueue.init() || !outputQueue.init()){
        return false;
    }

    running = true;
    for(auto &thread : writers){
        // TODO ID, err check, waitOnStop
        thread = sys_thread_new("WriterThread", writerThreadFunction, this, Config::THREAD_POOL_WRITER_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
    }

    for(auto &thread : readers){
        // TODO ID, err check, waitOnStop
        thread = sys_thread_new("ReaderThread", readerThreadFunction, this, Config::THREAD_POOL_READER_STACKSIZE, Config::THREAD_POOL_READER_PRIO);
    }
    return true;
}

void ThreadPool::stopThreads() {
    running = false;
}

void ThreadPool::clearQueues(){
    inputQueue.clear();
    outputQueue.clear();
}

void ThreadPool::addWorkload(Workload_t workload){
    inputQueue.moveElementIntoBuffer(std::move(workload));
}

void ThreadPool::writerThreadFunction(void* arg){
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
        printf("nullptr passed to writer function\n");
        return;
    }

    pool->doWriterWork();
}

void ThreadPool::doWriterWork(){
    while(running){
        Workload_t workload;
        auto isWorkToDo = inputQueue.moveFirstInto(workload);
        if(!isWorkToDo){
            sys_msleep(1);
            continue;
        }

        workload.p_writer->progress();
    }
}

void ThreadPool::readCallback(void* args, udp_pcb* target, pbuf* pbuf, const ip_addr_t* addr, Ip4Port_t port) {
    printf("Received something from %s:%u !!!!\n\r", ipaddr_ntoa(addr), port);

    auto& pool = *static_cast<ThreadPool*>(args);

    PacketInfo packet;
    packet.destAddr = {0}; // not relevant
    packet.destPort = target->local_port;
    packet.srcPort = port;
    packet.buffer = PBufWrapper{pbuf};
    pool.outputQueue.moveElementIntoBuffer(std::move(packet));
}


void ThreadPool::readerThreadFunction(void* arg){
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
        printf("nullptr passed to reader function\n");
        return;
    }
        pool->doReaderWork();
}

void ThreadPool::doReaderWork(){

    while(running){
        PacketInfo packet;
        auto isWorkToDo = outputQueue.moveFirstInto(packet);
        if(!isWorkToDo){

            sys_msleep(1);
            continue;
        }
        domain.receiveCallback(const_cast<const PacketInfo&>(packet));
    }
}
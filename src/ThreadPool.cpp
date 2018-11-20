/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/ThreadPool.h"
#include "lwip/tcpip.h"
#include "rtps/entities/Domain.h"

using rtps::ThreadPool;

ThreadPool::ThreadPool(Domain& domain) : domain(domain){

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
        // TODO ID, err check
        thread = sys_thread_new("WriterThread", writerFunction, this, Config::THREAD_POOL_WRITER_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
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

bool ThreadPool::addConnection(const ip4_addr_t& addr, const ip4_port_t port) {
    return transport.createUdpConnection(addr, port, readCallback, this);
}

void ThreadPool::addWorkload(Workload_t workload){
    inputQueue.moveElementIntoBuffer(std::move(workload));
}

void ThreadPool::writerFunction(void* arg){
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
        printf("nullptr passed to writer function\n");
        return;
    }
    while(pool->running){
        {
            Workload_t workload;
            auto isWorkToDo = pool->inputQueue.moveFirstInto(workload);
            if(!isWorkToDo){
                sys_msleep(1);
                continue;
            }
            for(uint8_t i=0; i < workload.numCacheChangesToSend; ++i){
                PBufWrapper buffer;
                workload.pWriter->createMessageCallback(buffer);

                if(!buffer.isValid()){
                    continue;
                }

                pool->outputQueue.moveElementIntoBuffer(std::move(buffer));

                // Execute with tcpip-thread
                tcpip_callback(sendFunction, pool); // Blocking i.e. thread safe call
            }
        }
    }
}

void ThreadPool::sendFunction(void* arg) {
    auto pool = static_cast<ThreadPool*>(arg);
    if(pool == nullptr){
        printf("nullptr passed to send function\n");
        return;
    }
    PBufWrapper pBufWrapper;
    const bool isWorkToDo = pool->outputQueue.moveFirstInto(pBufWrapper);
    if(!isWorkToDo){
        printf("Who dares to wake me up if there is nothing to do?!");
        return;
    }
    auto conn = pool->transport.createUdpConnection(pBufWrapper.addr, pBufWrapper.port, readCallback, pool);
    if(conn == nullptr){
        printf("Failed to create connection: %s:%u ", ipaddr_ntoa(&pBufWrapper.addr), pBufWrapper.port);
        return;
    }

    pool->transport.sendPacket(*conn, *pBufWrapper.firstElement);
}


void ThreadPool::readCallback(void* args, udp_pcb*, pbuf* pbuf, const ip_addr_t* addr, ip4_port_t port) {
    auto& pool = *static_cast<ThreadPool*>(args);

    printf("Received something from %s:%u !!!!\n\r", ipaddr_ntoa(addr), port);

    PBufWrapper wrapper{pbuf};
    wrapper.addr = *addr;
    wrapper.port = port;

    // TODO Other threads shall execute this
    pool.domain.receiveCallback(static_cast<const PBufWrapper>(wrapper)); // Avoid non-const use (API change might need this)
}
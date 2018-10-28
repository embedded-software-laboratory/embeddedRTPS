/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/ThreadPool.h"

class Lock{
public:
    Lock(sys_mutex_t& passedMutex) : mutex(passedMutex){
        sys_mutex_lock(&mutex);
    };
    ~Lock(){
        sys_mutex_unlock(&mutex);
    };
private:
    sys_mutex_t& mutex;
};

bool ThreadPool::startThreads(){
    if(running){
        return true;
    }
    if (sys_mutex_new(&queueMutex) != ERR_OK) {
        return false;
    }
    head = 0;
    tail = 0;
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

bool ThreadPool::addConnection(ip4_addr_t &addr, ip4_port_t port) {
    return transportDriver.createUdpConnection(addr, port, callback);
}

void ThreadPool::addWorkload(Workload_t work){
    Lock lock(queueMutex);
    workloadQueue[head] = work;
    head = (head+1) % workloadQueue.size();
}

void ThreadPool::writerFunction(void *arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while(pool->running){
        {
            Lock lock(pool->queueMutex);
            if(pool->head != pool->tail){
                Workload_t& current = pool->workloadQueue[pool->tail];
                pool->tail = (pool->tail+1) % pool->workloadQueue.size();
                pool->transportDriver.sendPacket(current.addr, current.port, current.data, current.size);
            }
        }
        sys_msleep(10);
    }
}


void ThreadPool::callback(void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, ip4_port_t port) {
    printf("Received something from %s:%u !!!!\n\r", ipaddr_ntoa(addr), port);
    for(int i=0; i < p->len; i++){
        printf("%c ", ((unsigned char*)p->payload)[i]);
    }
    printf("\n");
    pbuf_free(p);
}
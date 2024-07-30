/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#include "rtps/ThreadPool.h"

#include "lwip/tcpip.h"
#include "rtps/entities/Domain.h"
#include "rtps/entities/Writer.h"
#include "rtps/utils/Diagnostics.h"
#include "rtps/utils/Log.h"
#include "rtps/utils/udpUtils.h"

using rtps::ThreadPool;

#if THREAD_POOL_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef THREAD_POOL_LOG
#include "rtps/utils/strutils.h"
#define THREAD_POOL_LOG(...)                                                   \
  if (true) {                                                                  \
    printf("[ThreadPool] ");                                                   \
    printf(__VA_ARGS__);                                                       \
    printf("\r\n");                                                            \
  }
#endif
#else
#define THREAD_POOL_LOG(...) //
#endif

ThreadPool::ThreadPool(receiveJumppad_fp receiveCallback, void *callee)
    : m_receiveJumppad(receiveCallback), m_callee(callee) {

  if (!m_outgoingMetaTraffic.init() || !m_outgoingUserTraffic.init() ||
      !m_incomingMetaTraffic.init() || !m_incomingUserTraffic.init()) {
    return;
  }
  err_t inputErr = sys_sem_new(&m_readerNotificationSem, 0);
  err_t outputErr = sys_sem_new(&m_writerNotificationSem, 0);

  if (inputErr != ERR_OK || outputErr != ERR_OK) {
    THREAD_POOL_LOG("ThreadPool: Failed to create Semaphores.\n");
  }
}

ThreadPool::~ThreadPool() {
  if (m_running) {
    stopThreads();
    sys_msleep(500);
  }

  if (sys_sem_valid(&m_readerNotificationSem)) {
    sys_sem_free(&m_readerNotificationSem);
  }
  if (sys_sem_valid(&m_writerNotificationSem)) {
    sys_sem_free(&m_writerNotificationSem);
  }
}

void ThreadPool::updateDiagnostics() {

  rtps::Diagnostics::ThreadPool::max_ever_elements_incoming_usertraffic_queue =
      std::max(rtps::Diagnostics::ThreadPool::
                   max_ever_elements_incoming_usertraffic_queue,
               m_incomingUserTraffic.numElements());

  rtps::Diagnostics::ThreadPool::max_ever_elements_outgoing_usertraffic_queue =
      std::max(rtps::Diagnostics::ThreadPool::
                   max_ever_elements_outgoing_usertraffic_queue,
               m_outgoingUserTraffic.numElements());

  rtps::Diagnostics::ThreadPool::max_ever_elements_incoming_metatraffic_queue =
      std::max(rtps::Diagnostics::ThreadPool::
                   max_ever_elements_incoming_metatraffic_queue,
               m_incomingMetaTraffic.numElements());

  rtps::Diagnostics::ThreadPool::max_ever_elements_outgoing_metatraffic_queue =
      std::max(rtps::Diagnostics::ThreadPool::
                   max_ever_elements_outgoing_metatraffic_queue,
               m_outgoingMetaTraffic.numElements());
}

bool ThreadPool::startThreads() {
  if (m_running) {
    return true;
  }
  if (!sys_sem_valid(&m_readerNotificationSem) ||
      !sys_sem_valid(&m_writerNotificationSem)) {
    return false;
  }

  m_running = true;
  for (auto &thread : m_writers) {
    // TODO ID, err check, waitOnStop
    thread = sys_thread_new("WriterThread", writerThreadFunction, this,
                            Config::THREAD_POOL_WRITER_STACKSIZE,
                            Config::THREAD_POOL_WRITER_PRIO);
  }

  for (auto &thread : m_readers) {
    // TODO ID, err check, waitOnStop
    thread = sys_thread_new("ReaderThread", readerThreadFunction, this,
                            Config::THREAD_POOL_READER_STACKSIZE,
                            Config::THREAD_POOL_READER_PRIO);
  }
  return true;
}

void ThreadPool::stopThreads() {
  m_running = false;
  // This should call all the semaphores for each thread once, so they don't
  // stuck before ended.
  for (auto &thread : m_writers) {
    (void)thread;
    sys_sem_signal(&m_writerNotificationSem);
    sys_msleep(10);
  }
  for (auto &thread : m_readers) {
    (void)thread;
    sys_sem_signal(&m_readerNotificationSem);
    sys_msleep(10);
  }
  // TODO make sure they have finished. Seems to be sufficient for tests.
  // Not sufficient if threads shall actually be stopped during runtime.
  sys_msleep(10);
}

void ThreadPool::clearQueues() {
  m_outgoingMetaTraffic.clear();
  m_outgoingUserTraffic.clear();
  m_incomingMetaTraffic.clear();
  m_incomingUserTraffic.clear();
}

bool ThreadPool::addWorkload(Writer *workload) {
  bool res = false;
  if (workload->isBuiltinEndpoint()) {
    res = m_outgoingMetaTraffic.moveElementIntoBuffer(std::move(workload));
  } else {
    res = m_outgoingUserTraffic.moveElementIntoBuffer(std::move(workload));
  }
  if (res) {
    sys_sem_signal(&m_writerNotificationSem);
  } else {
	if(workload->isBuiltinEndpoint()){
		rtps::Diagnostics::ThreadPool::dropped_outgoing_packets_metatraffic++;
	}else{
		rtps::Diagnostics::ThreadPool::dropped_outgoing_packets_usertraffic++;
	}
    THREAD_POOL_LOG("Failed to enqueue outgoing packet.");
  }

  return res;
}

bool ThreadPool::addBuiltinPort(const Ip4Port_t &port) {
  if (m_builtinPortsIdx == m_builtinPorts.size()) {
    return false;
  }

  // TODO: Does not allow for participant deletion!
  m_builtinPorts[m_builtinPortsIdx] = port;
  m_builtinPortsIdx++;

  return true;
}

bool ThreadPool::isBuiltinPort(const Ip4Port_t &port) {
  if (getBuiltInMulticastLocator().port == port) {
    return true;
  }

  for (unsigned int i = 0; i < m_builtinPortsIdx; i++) {
    if (m_builtinPorts[i] == port) {
      return true;
    }
  }

  return false;
}

bool ThreadPool::addNewPacket(PacketInfo &&packet) {
  bool res = false;
  if (isBuiltinPort(packet.destPort)) {
    res = m_incomingMetaTraffic.moveElementIntoBuffer(std::move(packet));
  } else {
    res = m_incomingUserTraffic.moveElementIntoBuffer(std::move(packet));
  }
  if (res) {
    sys_sem_signal(&m_readerNotificationSem);
  } else {
    THREAD_POOL_LOG("failed to enqueue packet for port %u",
                    static_cast<unsigned int>(packet.destPort));
  }
  return res;
}

void ThreadPool::writerThreadFunction(void *arg) {
  auto pool = static_cast<ThreadPool *>(arg);
  if (pool == nullptr) {

    THREAD_POOL_LOG("nullptr passed to writer function\n");

    return;
  }

  pool->doWriterWork();
}

void ThreadPool::doWriterWork() {
  while (m_running) {
    Writer *workload_usertraffic = nullptr;
    bool workload_usertraffic_available = m_outgoingUserTraffic.moveFirstInto(workload_usertraffic);
    if (workload_usertraffic_available) {
      workload_usertraffic->progress();
      Diagnostics::ThreadPool::processed_outgoing_usertraffic++;
    }

    Writer *workload_metatraffic = nullptr;
    bool workload_metatraffic_available = m_outgoingMetaTraffic.moveFirstInto(workload_metatraffic);
    if (workload_metatraffic_available) {
      workload_metatraffic->progress();
      Diagnostics::ThreadPool::processed_outgoing_metatraffic++;
    }

    if (workload_usertraffic_available || workload_metatraffic_available) {
      continue;
    } else {
      THREAD_POOL_LOG("WriterWorker | User = %u, Meta = %u\r\n",
                      static_cast<unsigned int>(Diagnostics::ThreadPool::processed_outgoing_usertraffic),
                      static_cast<unsigned int>(Diagnostics::ThreadPool::processed_outgoing_metatraffic));
      updateDiagnostics();
      sys_sem_wait(&m_writerNotificationSem);
    }
  }
}

void ThreadPool::readCallback(void *args, udp_pcb *target, pbuf *pbuf,
                              const ip_addr_t *addr, Ip4Port_t port) {
  auto &pool = *static_cast<ThreadPool *>(args);

  PacketInfo packet;

  // TODO This is a workaround for chained pbufs caused by hardware limitations,
  // not a general fix
  if (pbuf->next != nullptr) {
    struct pbuf *test = pbuf_alloc(PBUF_RAW, pbuf->tot_len, PBUF_POOL);
    pbuf_copy(test, pbuf);
    pbuf_free(pbuf);
    pbuf = test;
  }

  packet.destAddr = {0}; // not relevant
  packet.destPort = target->local_port;
  packet.srcPort = port;
  packet.buffer = PBufWrapper{pbuf};

  if (!pool.addNewPacket(std::move(packet))) {
    THREAD_POOL_LOG("ThreadPool: dropped packet\n");
    if (pool.isBuiltinPort(port)) {
      rtps::Diagnostics::ThreadPool::dropped_incoming_packets_metatraffic++;
    } else {
      rtps::Diagnostics::ThreadPool::dropped_incoming_packets_usertraffic++;
    }
  }
}

void ThreadPool::readerThreadFunction(void *arg) {
  auto pool = static_cast<ThreadPool *>(arg);
  if (pool == nullptr) {

    THREAD_POOL_LOG("nullptr passed to reader function\n");

    return;
  }
  pool->doReaderWork();
}

void ThreadPool::doReaderWork() {
  uint32_t metatraffic = 0;
  uint32_t usertraffic = 0;
  while (m_running) {
    PacketInfo packet_user;
    auto isUserWorkToDo = m_incomingUserTraffic.moveFirstInto(packet_user);
    if (isUserWorkToDo) {
      Diagnostics::ThreadPool::processed_incoming_usertraffic++;
      m_receiveJumppad(m_callee, const_cast<const PacketInfo &>(packet_user));
    }

    PacketInfo packet_meta;
    auto isMetaWorkToDo = m_incomingMetaTraffic.moveFirstInto(packet_meta);
    if (isMetaWorkToDo) {
      Diagnostics::ThreadPool::processed_incoming_metatraffic++;
      m_receiveJumppad(m_callee, const_cast<const PacketInfo &>(packet_meta));
    }

    if (isUserWorkToDo || isMetaWorkToDo) {
      continue;
    }
    THREAD_POOL_LOG("ReaderWorker | User = %u, Meta = %u\r\n",
                    static_cast<unsigned int>(usertraffic),
                    static_cast<unsigned int>(metatraffic));
    updateDiagnostics();
    sys_sem_wait(&m_readerNotificationSem);
  }
}

#undef THREAD_POOL_VERBOSE

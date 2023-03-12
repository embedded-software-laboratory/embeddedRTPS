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

#ifndef RTPS_THREADPOOL_H
#define RTPS_THREADPOOL_H

#include "lwip/sys.h"
#include "rtps/communication/PacketInfo.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/config.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/ThreadSafeCircularBuffer.h"

#include <array>

namespace rtps {

class Writer;

class ThreadPool {
public:
  using receiveJumppad_fp = void (*)(void *callee, const PacketInfo &packet);

  ThreadPool(receiveJumppad_fp receiveCallback, void *callee);

  ~ThreadPool();

  bool startThreads();
  void stopThreads();

  void clearQueues();
  bool addWorkload(Writer *workload);
  bool addNewPacket(PacketInfo &&packet);

  static void readCallback(void *arg, udp_pcb *pcb, pbuf *p,
                           const ip_addr_t *addr, Ip4Port_t port);

  bool addBuiltinPort(const Ip4Port_t &port);

private:
  receiveJumppad_fp m_receiveJumppad;
  void *m_callee;
  bool m_running = false;
  std::array<sys_thread_t, Config::THREAD_POOL_NUM_WRITERS> m_writers;
  std::array<sys_thread_t, Config::THREAD_POOL_NUM_READERS> m_readers;

  std::array<Ip4Port_t, 2 * Config::MAX_NUM_PARTICIPANTS> m_builtinPorts;
  size_t m_builtinPortsIdx = 0;

  sys_sem_t m_readerNotificationSem;
  sys_sem_t m_writerNotificationSem;

  void updateDiagnostics();

  using BufferUsertrafficOutgoing = ThreadSafeCircularBuffer<
      Writer *, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH_USERTRAFFIC>;
  using BufferMetatrafficOutgoing = ThreadSafeCircularBuffer<
      Writer *, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH_METATRAFFIC>;
  using BufferUsertrafficIncoming = ThreadSafeCircularBuffer<
      PacketInfo, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH_USERTRAFFIC>;
  using BufferMetatrafficIncoming = ThreadSafeCircularBuffer<
      PacketInfo, Config::THREAD_POOL_WORKLOAD_QUEUE_LENGTH_METATRAFFIC>;

  BufferUsertrafficOutgoing m_outgoingUserTraffic;
  BufferMetatrafficOutgoing m_outgoingMetaTraffic;

  BufferUsertrafficIncoming m_incomingUserTraffic;
  BufferMetatrafficIncoming m_incomingMetaTraffic;

  bool isBuiltinPort(const Ip4Port_t &port);
  static void writerThreadFunction(void *arg);
  static void readerThreadFunction(void *arg);
  void doWriterWork();
  void doReaderWork();
};
} // namespace rtps

#endif // RTPS_THREADPOOL_H

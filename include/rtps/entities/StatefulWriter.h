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

#ifndef RTPS_STATEFULWRITER_H
#define RTPS_STATEFULWRITER_H

#include "rtps/entities/ReaderProxy.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/HistoryCacheWithDeletion.h"
#include "rtps/storages/MemoryPool.h"

namespace rtps {

template <class NetworkDriver> class StatefulWriterT final : public Writer {
public:
  ~StatefulWriterT() override;
  bool init(TopicData attributes, TopicKind_t topicKind, ThreadPool *threadPool,
            NetworkDriver &driver, bool enfUnicast = false);

  //! Executes required steps like sending packets. Intended to be called by
  //! worker threads
  void progress() override;
  const CacheChange *newChange(ChangeKind_t kind, const uint8_t *data,
                               DataSize_t size, bool inLineQoS = false,
                               bool markDisposedAfterWrite = false) override;

  bool removeFromHistory(const SequenceNumber_t &s);
  void setAllChangesToUnsent() override;
  void onNewAckNack(const SubmessageAckNack &msg,
                    const GuidPrefix_t &sourceGuidPrefix) override;
  void reset() override;
  void updateChangeKind(SequenceNumber_t &sequence_number);

private:
  NetworkDriver *m_transport;

  HistoryCacheWithDeletion<Config::HISTORY_SIZE_STATEFUL> m_history;

  /*
   * Cache changes marked as disposeAfterWrite are retained for a short amount in case of retransmission
   * The whole 'disposeAfterWrite' mechanisms only exists to allow for repeated creation and deletion of endpoints during operation.
   * Otherwise the history will quickly reach its limits.
   * Will be replaced with something more elegant in the future.
   */
  ThreadSafeCircularBuffer<SequenceNumber_t, 10> m_disposeWithDelay;
  void dropDisposeAfterWriteChanges();
  
  sys_thread_t m_heartbeatThread;

  Count_t m_hbCount{1};

  bool m_running = true;
  bool m_thread_running = false;

  bool sendData(const ReaderProxy &reader, const CacheChange *next);
  bool sendDataWRMulticast(const ReaderProxy &reader, const CacheChange *next);
  static void hbFunctionJumppad(void *thisPointer);
  void sendHeartBeatLoop();
  void sendHeartBeat();
  void sendGap(const ReaderProxy &reader, const SequenceNumber_t& firstMissing, const SequenceNumber_t& nextValid);
};

using StatefulWriter = StatefulWriterT<UdpDriver>;
} // namespace rtps

#include "StatefulWriter.tpp"

#endif // RTPS_STATEFULWRITER_H

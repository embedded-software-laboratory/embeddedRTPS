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
#include "rtps/storages/MemoryPool.h"
#include "rtps/storages/SimpleHistoryCache.h"

namespace rtps {

template <class NetworkDriver> class StatefulWriterT final : public Writer {
public:
  ~StatefulWriterT() override;
  bool init(TopicData attributes, TopicKind_t topicKind, ThreadPool *threadPool,
            NetworkDriver &driver, bool enfUnicast = false);

  bool addNewMatchedReader(const ReaderProxy &newProxy) override;
  void removeReader(const Guid &guid) override;
  void removeReaderOfParticipant(const GuidPrefix_t &guidPrefix) override;
  //! Executes required steps like sending packets. Intended to be called by
  //! worker threads
  void progress() override;
  const CacheChange *newChange(ChangeKind_t kind, const uint8_t *data,
                               DataSize_t size) override;
  void setAllChangesToUnsent() override;
  void onNewAckNack(const SubmessageAckNack &msg,
                    const GuidPrefix_t &sourceGuidPrefix) override;

private:
  sys_mutex_t m_mutex;
  ThreadPool *mp_threadPool = nullptr;

  PacketInfo m_packetInfo;
  NetworkDriver *m_transport;
  bool m_enforceUnicast;

  TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
  SequenceNumber_t m_nextSequenceNumberToSend = {0, 1};
  SimpleHistoryCache m_history;
  sys_thread_t m_heartbeatThread;
  Count_t m_hbCount{1};

  bool m_running = true;


  bool sendData(const ReaderProxy &reader, const SequenceNumber_t &sn);
  bool sendDataWRMulticast(const ReaderProxy &reader, const SequenceNumber_t &sn);
  static void hbFunctionJumppad(void *thisPointer);
  void sendHeartBeatLoop();
  void sendHeartBeat();
  bool isIrrelevant(ChangeKind_t kind) const;
  void manageSendOptions();
  void resetSendOptions();
};

using StatefulWriter = StatefulWriterT<UdpDriver>;
} // namespace rtps

#include "StatefulWriter.tpp"

#endif // RTPS_STATEFULWRITER_H

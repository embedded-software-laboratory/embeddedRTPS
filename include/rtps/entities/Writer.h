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

#ifndef RTPS_WRITER_H
#define RTPS_WRITER_H

#include "rtps/ThreadPool.h"
#include "rtps/discovery/TopicData.h"
#include "rtps/entities/ReaderProxy.h"
#include "rtps/storages/CacheChange.h"
#include "rtps/storages/MemoryPool.h"
#include "rtps/storages/PBufWrapper.h"

#ifdef DEBUG_BUILD
#define COMPILE_INIT_GUARD
#endif

#ifdef COMPILE_INIT_GUARD
#define INIT_GUARD()                                                           \
  if (!m_is_initialized_) {                                                    \
    while (1) {                                                                \
      printf("using uninitalized endpoint\n;");                                \
    }                                                                          \
  }
#else
#define INIT_GUARD() //
#endif

namespace rtps {

class Writer {
public:
  TopicData m_attributes;
  virtual bool addNewMatchedReader(const ReaderProxy &newProxy);
  virtual bool removeProxy(const Guid_t &guid);
  virtual void removeAllProxiesOfParticipant(const GuidPrefix_t &guidPrefix);
  virtual void reset() = 0;
  virtual const CacheChange *newChange(ChangeKind_t kind, const uint8_t *data,
                                       DataSize_t size);

  //! Executes required steps like sending packets. Intended to be called by
  //! worker threads
  virtual void progress() = 0;

  virtual bool removeFromHistory(const SequenceNumber_t &s) = 0;
  virtual void setAllChangesToUnsent() = 0;
  virtual void onNewAckNack(const SubmessageAckNack &msg,
                            const GuidPrefix_t &sourceGuidPrefix) = 0;

  using dumpProxyCallback = void (*)(const Writer *writer, const ReaderProxy &,
                                     void *arg);

  int dumpAllProxies(dumpProxyCallback target, void *arg);

  bool isInitialized();
  uint32_t getProxiesCount();

  void setSEDPSequenceNumber(const SequenceNumber_t &sn);
  const SequenceNumber_t &getSEDPSequenceNumber();

protected:
  SequenceNumber_t m_sedp_sequence_number;

  Lock_t m_mutex = nullptr;
  ThreadPool *mp_threadPool = nullptr;

  Ip4Port_t m_srcPort;

  bool m_enforceUnicast;

  TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
  SequenceNumber_t m_nextSequenceNumberToSend;

  friend class SEDPAgent;
  virtual const CacheChange *newChange(ChangeKind_t kind, const uint8_t *data,
                                       DataSize_t size, bool inLineQoS,
                                       bool markDisposedAfterWrite) = 0;

  friend class SizeInspector;
  bool m_is_initialized_ = false;
  virtual ~Writer() = default;
  MemoryPool<ReaderProxy, Config::NUM_READER_PROXIES_PER_WRITER> m_proxies;

  void resetSendOptions();
  void manageSendOptions();
  bool isIrrelevant(ChangeKind_t kind) const;
};
} // namespace rtps

#endif // RTPS_WRITER_H

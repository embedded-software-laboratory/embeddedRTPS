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

#ifndef RTPS_READER_H
#define RTPS_READER_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/TopicData.h"
#include "rtps/entities/WriterProxy.h"
#include "rtps/storages/MemoryPool.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/Lock.h"
#include <cstring>

namespace rtps {

struct SubmessageHeartbeat;
struct SubmessageGap;

class ReaderCacheChange {
private:
  const uint8_t *data;

public:
  const ChangeKind_t kind;
  const DataSize_t size;
  const Guid_t writerGuid;
  const SequenceNumber_t sn;

  ReaderCacheChange(ChangeKind_t kind, Guid_t &writerGuid, SequenceNumber_t sn,
                    const uint8_t *data, DataSize_t size)
      : data(data), kind(kind), size(size), writerGuid(writerGuid), sn(sn){};

  ~ReaderCacheChange() =
      default; // No need to free data. It's not owned by this object
  // Not allowed because this class doesn't own the ptr and the user isn't
  // allowed to use it outside the Scope of the callback
  ReaderCacheChange(const ReaderCacheChange &other) = delete;
  ReaderCacheChange(ReaderCacheChange &&other) = delete;
  ReaderCacheChange &operator=(const ReaderCacheChange &other) = delete;
  ReaderCacheChange &operator=(ReaderCacheChange &&other) = delete;

  bool copyInto(uint8_t *buffer, DataSize_t destSize) const {
    if (destSize < size) {
      return false;
    } else {
      memcpy(buffer, data, size);
      return true;
    }
  }

  const uint8_t *getData() const { return data; }

  const DataSize_t getDataSize() const { return size; }
};

typedef void (*ddsReaderCallback_fp)(void *callee,
                                     const ReaderCacheChange &cacheChange);

class Reader {
public:
  using callbackFunction_t = void (*)(void *, const ReaderCacheChange &);
  using callbackIdentifier_t = uint32_t;

  TopicData m_attributes;
  virtual void newChange(const ReaderCacheChange &cacheChange) = 0;
  virtual callbackIdentifier_t registerCallback(callbackFunction_t cb,
                                                void *arg);
  virtual bool removeCallback(callbackIdentifier_t identifier);
  uint8_t getNumCallbacks();

  virtual bool onNewHeartbeat(const SubmessageHeartbeat &msg,
                              const GuidPrefix_t &remotePrefix) = 0;
  virtual bool onNewGapMessage(const SubmessageGap &msg,
                               const GuidPrefix_t &remotePrefix) = 0;
  virtual bool addNewMatchedWriter(const WriterProxy &newProxy) = 0;
  virtual bool removeProxy(const Guid_t &guid);
  virtual void removeAllProxiesOfParticipant(const GuidPrefix_t &guidPrefix);
  bool isInitialized() { return m_is_initialized_; }
  virtual void reset();
  bool isProxy(const Guid_t &guid);
  WriterProxy *getProxy(Guid_t guid);
  uint32_t getProxiesCount();

  void setSEDPSequenceNumber(const SequenceNumber_t &sn);
  const SequenceNumber_t &getSEDPSequenceNumber();

  using dumpProxyCallback = void (*)(const Reader *reader, const WriterProxy &,
                                     void *arg);

  //! Dangerous, only
  int dumpAllProxies(dumpProxyCallback target, void *arg);

protected:
  void executeCallbacks(const ReaderCacheChange &cacheChange);
  bool initMutex();

  SequenceNumber_t m_sedp_sequence_number;

  bool m_is_initialized_ = false;
  Reader();
  virtual ~Reader() = default;
  MemoryPool<WriterProxy, Config::NUM_WRITER_PROXIES_PER_READER> m_proxies;

  callbackIdentifier_t m_callback_identifier = 1;

  uint8_t m_callback_count = 0;
  using callbackElement_t = struct {
    callbackFunction_t function;
    void *arg;
    callbackIdentifier_t identifier;
  };

  std::array<callbackElement_t, Config::MAX_NUM_READER_CALLBACKS> m_callbacks;

  // Guards manipulation of the proxies array
  Lock_t m_proxies_mutex = nullptr;

  // Guards manipulation of callback array
  Lock_t m_callback_mutex = nullptr;
};
} // namespace rtps

#endif // RTPS_READER_H

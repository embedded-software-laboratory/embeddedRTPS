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
#include <cstring>

namespace rtps {

struct SubmessageHeartbeat;

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
  TopicData m_attributes;
  virtual void newChange(const ReaderCacheChange &cacheChange) = 0;
  virtual void registerCallback(ddsReaderCallback_fp cb, void *callee) = 0;
  virtual bool onNewHeartbeat(const SubmessageHeartbeat &msg,
                              const GuidPrefix_t &remotePrefix) = 0;
  virtual bool addNewMatchedWriter(const WriterProxy &newProxy) = 0;
  virtual void removeWriter(const Guid_t &guid) = 0;
  virtual void removeWriterOfParticipant(const GuidPrefix_t &guidPrefix) = 0;
  bool isInitialized() { return m_is_initialized_; }

  bool knowWriterId(const Guid_t &guid) {
    for (const auto &proxy : m_proxies) {
      if (proxy.remoteWriterGuid.operator==(guid)) {
        return true;
      }
    }
    return false;
  }

  uint32_t getNumMatchedWriters() { return m_proxies.getSize(); }

protected:
  bool m_is_initialized_ = false;
  virtual ~Reader() = default;
  MemoryPool<WriterProxy, Config::NUM_WRITER_PROXIES_PER_READER> m_proxies;
};
} // namespace rtps

#endif // RTPS_READER_H

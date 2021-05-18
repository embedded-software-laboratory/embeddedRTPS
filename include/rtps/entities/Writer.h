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

namespace rtps {

class Writer {
public:
  TopicData m_attributes;
  virtual bool addNewMatchedReader(const ReaderProxy &newProxy) = 0;
  virtual void removeReader(const Guid_t &guid) = 0;
  virtual void removeReaderOfParticipant(const GuidPrefix_t &guidPrefix) = 0;

  //! Executes required steps like sending packets. Intended to be called by
  //! worker threads
  virtual void progress() = 0;
  virtual const CacheChange *newChange(ChangeKind_t kind, const uint8_t *data,
                                       DataSize_t size) = 0;
  virtual void setAllChangesToUnsent() = 0;
  virtual void onNewAckNack(const SubmessageAckNack &msg,
                            const GuidPrefix_t &sourceGuidPrefix) = 0;

  bool isInitialized() { return m_is_initialized_; }

protected:
  bool m_is_initialized_ = false;
  virtual ~Writer() = default;
  MemoryPool<ReaderProxy, Config::NUM_READER_PROXIES_PER_WRITER> m_proxies;
};
} // namespace rtps

#endif // RTPS_WRITER_H

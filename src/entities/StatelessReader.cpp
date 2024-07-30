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

#include "rtps/entities/StatelessReader.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

using rtps::StatelessReader;

#if SLR_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/strutils.h"
#ifndef SLR_LOG
#define SLR_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatelessReader %s] ", &m_attributes.topicName[0]);               \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#endif
#else
#define SLR_LOG(...) //
#endif

bool StatelessReader::init(const TopicData &attributes) {
  if (!initMutex()) {
    return false;
  }

  m_proxies.clear();
  m_attributes = attributes;
  m_is_initialized_ = true;
  return true;
}

void StatelessReader::newChange(const ReaderCacheChange &cacheChange) {
  if (!m_is_initialized_) {
    return;
  }
  executeCallbacks(cacheChange);
}

bool StatelessReader::addNewMatchedWriter(const WriterProxy &newProxy) {
#if (SLR_VERBOSE && RTPS_GLOBAL_VERBOSE)
  char buffer[64];
  guid2Str(newProxy.remoteWriterGuid, buffer, sizeof(buffer));
  SLR_LOG("Adding WriterProxy: %s", buffer);
#endif
  return m_proxies.add(newProxy);
}

bool StatelessReader::onNewHeartbeat(const SubmessageHeartbeat &,
                                     const GuidPrefix_t &) {
  // nothing to do
  return true;
}

bool StatelessReader::onNewGapMessage(const SubmessageGap &msg,
                                      const GuidPrefix_t &remotePrefix) {
  return true;
}

#undef SLR_VERBOSE

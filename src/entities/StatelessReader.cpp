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

using rtps::StatelessReader;

#define SLR_VERBOSE 0

void StatelessReader::init(const TopicData &attributes) {
  m_attributes = attributes;
  m_is_initialized_ = true;
}

void StatelessReader::newChange(const ReaderCacheChange &cacheChange) {
  if (m_callback != nullptr) {
    m_callback(m_callee, cacheChange);
  }
}

void StatelessReader::registerCallback(ddsReaderCallback_fp cb, void *callee) {
  if (cb != nullptr) {
    m_callback = cb;
    m_callee = callee; // It's okay if this is null
  } else {
#if SLR_VERBOSE
    printf("StatelessReader[%s]: Passed callback is nullptr\n",
           &m_attributes.topicName[0]);
#endif
  }
}

bool StatelessReader::addNewMatchedWriter(const WriterProxy &newProxy) {
  return m_proxies.add(newProxy);
}

void StatelessReader::removeWriter(const Guid & /*guid*/) {
  // Nothing to do
}

bool StatelessReader::onNewHeartbeat(const SubmessageHeartbeat &,
                                     const GuidPrefix_t &) {
  // nothing to do
  return true;
}

#undef SLR_VERBOSE

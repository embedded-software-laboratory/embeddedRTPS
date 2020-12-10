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

#include "rtps/storages/HistoryCache.h"

using rtps::HistoryCache;

const rtps::CacheChange *HistoryCache::addChange(CacheChange &&newChange) {
  auto &change = m_buffer[m_head];
  change = std::move(newChange);

  incrementHead();

  return &change;
}

void HistoryCache::dropFirst() { incrementTail(); }

bool HistoryCache::isFull() const {
  auto iterator = m_head;
  incrementIterator(iterator);
  return iterator == m_tail;
}

const rtps::SequenceNumber_t &HistoryCache::getSeqNumMin() const {
  const SequenceNumber_t *pSN = &SEQUENCENUMBER_UNKNOWN;
  auto iterator = m_tail;
  while (iterator != m_head) {
    auto &change = m_buffer[iterator];
    if (pSN == &SEQUENCENUMBER_UNKNOWN || change.sequenceNumber < *pSN) {
      pSN = &change.sequenceNumber;
    }
    incrementIterator(iterator);
  }

  return *pSN;
}

const rtps::SequenceNumber_t &HistoryCache::getSeqNumMax() const {
  const SequenceNumber_t *pSN = &SEQUENCENUMBER_UNKNOWN;

  auto iterator = m_tail;
  while (iterator != m_head) {
    auto &change = m_buffer[iterator];
    if (pSN == &SEQUENCENUMBER_UNKNOWN || *pSN < change.sequenceNumber) {
      pSN = &change.sequenceNumber;
    }
    incrementIterator(iterator);
  }
  return *pSN;
}

const rtps::CacheChange *
HistoryCache::getChangeBySN(const SequenceNumber_t &sn) const {
  auto iterator = m_tail;
  while (iterator != m_head) {
    auto &change = m_buffer[iterator];
    if (change.sequenceNumber == sn) {
      return &change;
    }
    incrementIterator(iterator);
  }

  return nullptr;
}

void HistoryCache::incrementHead() {
  incrementIterator(m_head);
  if (m_head == m_tail) {
    incrementTail();
  }
}

void HistoryCache::incrementTail() { incrementIterator(m_tail); }

void HistoryCache::incrementIterator(uint16_t &iterator) const {
  ++iterator;
  if (iterator >= m_buffer.size()) {
    iterator = 0;
  }
}

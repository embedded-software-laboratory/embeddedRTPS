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

#include "rtps/storages/SimpleHistoryCache.h"
#include <limits>

using rtps::SimpleHistoryCache;

SimpleHistoryCache::SimpleHistoryCache(SequenceNumber_t lastUsed)
    : SimpleHistoryCache() {
  m_lastUsedSequenceNumber = lastUsed;
}

bool SimpleHistoryCache::isFull() const {
  uint16_t it = m_head;
  incrementIterator(it);
  return it == m_tail;
}

const rtps::SequenceNumber_t &SimpleHistoryCache::getSeqNumMin() const {
  if (m_head == m_tail) {
    return SEQUENCENUMBER_UNKNOWN;
  } else {
    return m_buffer[m_tail].sequenceNumber;
  }
}

const rtps::SequenceNumber_t &SimpleHistoryCache::getSeqNumMax() const {
  if (m_head == m_tail) {
    return SEQUENCENUMBER_UNKNOWN;
  } else {
    return m_lastUsedSequenceNumber;
  }
}

const rtps::CacheChange *SimpleHistoryCache::addChange(const uint8_t *data,
                                                       DataSize_t size) {
  CacheChange change;
  change.kind = ChangeKind_t::ALIVE;
  change.data.reserve(size);
  change.data.append(data, size);
  change.sequenceNumber = ++m_lastUsedSequenceNumber;

  CacheChange *place = &m_buffer[m_head];
  incrementHead();

  *place = std::move(change);
  return place;
}

void SimpleHistoryCache::dropOldest() { removeUntilIncl(getSeqNumMin()); }

void SimpleHistoryCache::removeUntilIncl(SequenceNumber_t sn) {
  if (m_head == m_tail) {
    return;
  }

  if (getSeqNumMax() <= sn) { // We won't overrun head
    m_head = m_tail;
    return;
  }

  while (m_buffer[m_tail].sequenceNumber <= sn) {
    incrementTail();
  }
}

const rtps::CacheChange *
SimpleHistoryCache::getChangeBySN(SequenceNumber_t sn) const {
  SequenceNumber_t minSN = getSeqNumMin();
  if (sn < minSN || getSeqNumMax() < sn) {
    return nullptr;
  }
  static_assert(std::is_unsigned<decltype(sn.low)>::value,
                "Underflow well defined");
  static_assert(sizeof(m_tail) <= sizeof(uint16_t), "Cast ist well defined");
  // We don't overtake head, therefore difference of sn is within same range as
  // iterators
  uint16_t pos = m_tail + static_cast<uint16_t>(sn.low - minSN.low);

  // Diff is smaller than the size of the array -> max one overflow
  if (pos >= m_buffer.size()) {
    pos -= m_buffer.size();
  }
  return &m_buffer[pos];
}

void SimpleHistoryCache::incrementHead() {
  incrementIterator(m_head);
  if (m_head == m_tail) {
    // Move without check
    incrementIterator(m_tail); // drop one
  }
}

void SimpleHistoryCache::incrementTail() {
  if (m_head != m_tail) {
    incrementIterator(m_tail);
  }
}

void SimpleHistoryCache::incrementIterator(uint16_t &iterator) const {
  ++iterator;
  if (iterator >= m_buffer.size()) {
    iterator = 0;
  }
}

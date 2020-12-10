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

#ifndef RTPS_HISTORYCACHE_H
#define RTPS_HISTORYCACHE_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/storages/CacheChange.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps {

/**
 * This class can be used when we need to invalidate data in between or the
 * sequence numbers are not consecutive
 */
class HistoryCache {
public:
  /**
   * Adds a new Change. If the buffer is full, it will override
   * the oldest one. If you want to avoid this, use isFull() and dropFirst().
   */
  const CacheChange *addChange(CacheChange &&newChange);
  void dropFirst();
  bool isFull() const;

  const CacheChange *getChangeBySN(const SequenceNumber_t &sn) const;

  const SequenceNumber_t &getSeqNumMin() const;
  const SequenceNumber_t &getSeqNumMax() const;

private:
  std::array<CacheChange, Config::HISTORY_SIZE + 1> m_buffer{};
  uint16_t m_head = 0;
  uint16_t m_tail = 0;
  static_assert(sizeof(rtps::Config::HISTORY_SIZE) < sizeof(m_head),
                "Iterator is large enough for given size");

  inline void incrementHead();
  inline void incrementIterator(uint16_t &iterator) const;
  inline void incrementTail();
};
} // namespace rtps

#endif // RTPS_HISTORYCACHE_H

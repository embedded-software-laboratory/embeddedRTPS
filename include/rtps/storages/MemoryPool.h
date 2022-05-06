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

#ifndef RTPS_MEMORYPOOL_H
#define RTPS_MEMORYPOOL_H

#include <cstdint>
#include <cstring>
#include <iterator>

namespace rtps {

template <class TYPE, uint32_t SIZE> class MemoryPool {
public:
  template <typename IT_TYPE> class MemoryPoolIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = IT_TYPE;
    using difference_type = uint8_t;
    using pointer = IT_TYPE *;
    using reference = IT_TYPE &;

    explicit MemoryPoolIterator(MemoryPool<TYPE, SIZE> &pool) : m_pool(pool) {
      memcpy(m_bitMap, m_pool.m_bitMap, sizeof(m_bitMap));
    }

    // bool operator==(const MemoryPoolIterator& other) const{
    //    return bit == other.bit;
    //}

    bool operator!=(const MemoryPoolIterator &other) const {
      return m_bit != other.m_bit;
    }

    reference operator*() const { return m_pool.m_data[m_bit]; }

    reference operator->() const { return m_pool.m_data[m_bit]; }

    // Pre-increment
    MemoryPoolIterator &operator++() {
      if (m_pool.m_numElements == 0) {
        m_bit = SIZE;
        return *this;
      }
      uint32_t bucket;
      do {
        ++m_bit;
        bucket = m_bit / static_cast<uint8_t>(8);
      } while (!(m_bitMap[bucket] & (1 << (m_bit % 8))) && m_bit < SIZE);

      return *this;
    }

    // Post-increment
    MemoryPoolIterator operator++(int) {
      MemoryPoolIterator tmp(*this);
      ++(*this);
      return tmp;
    }

  private:
    friend class MemoryPool;
    MemoryPool<TYPE, SIZE> &m_pool;
    uint8_t m_bitMap[SIZE / 8 + 1];
    uint8_t m_bit = 0;
  };

  typedef MemoryPoolIterator<TYPE> MemPoolIter;
  typedef MemoryPoolIterator<const TYPE> const_MemPoolIter;

  typedef bool (*condition_fp)(TYPE);

  uint32_t getSize() { return SIZE; }

  bool isFull() { return m_numElements == SIZE; }

  bool isEmpty() { return m_numElements == 0; }

  uint32_t getNumElements() { return m_numElements; }

  bool add(const TYPE &data) {
    if (isFull()) {
      printf("[MemoryPool] RESSOURCE LIMIT EXCEEDED \n");
      return false;
    }
    for (uint8_t bucket = 0; bucket < sizeof(m_bitMap); ++bucket) {
      if (bucket != 0xFF) {
        uint8_t byte = m_bitMap[bucket];
        for (uint8_t bit = 0; bit < 8; ++bit) {
          if (!(byte & 1)) {
            m_bitMap[bucket] |= 1 << bit;
            m_data[bucket * 8 + bit] = data;
            ++m_numElements;
            return true;
          }
          byte = byte >> 1;
        }
      }
    }
    return false;
  }

  /**
   * Parameters are used in that way to allow lambdas with captures. Use this by
   * creating two: E.g.: auto callback=[data](TYPE& value){return value ==
   * data;}; auto thunk=[](void* arg, TYPE& value){return
   * (*static_cast<decltype(callback)*>(arg))(value);};
   *
   * and then simply call:
   * remove(thunk, &callback)
   *
   * NOTE: You have to make sure that the callback did not run out of scope.
   */
  bool remove(bool (*jumppad)(void *, const TYPE &data),
              void *isCorrectElement) {
    bool retcode = false;
    for (auto it = begin(); it != end(); ++it) {
      if (jumppad(isCorrectElement, *it)) {
        const uint8_t bucket = it.m_bit / uint8_t{8};
        const uint8_t pos =
            it.m_bit &
            uint8_t{
                7}; // 7 sets all bits above and including the one for 8 to 0
        m_bitMap[bucket] &= ~(static_cast<uint8_t>(1) << pos);
        --m_numElements;
        retcode = true;
      }
    }
    return retcode;
  }

  TYPE *find(bool (*jumppad)(void *, const TYPE &data),
             void *isCorrectElement) {
    for (auto it = begin(); it != end(); ++it) {
      if (jumppad(isCorrectElement, *it)) {
        return &(*it);
      }
    }
    return nullptr;
  }

  MemPoolIter begin() {
    MemPoolIter it(*this);
    if (!(m_bitMap[0] & 1)) {
      ++it;
    }
    return it;
  }

  MemPoolIter end() {
    MemPoolIter endIt(*this);
    endIt.m_bit = SIZE;
    return endIt;
  }

private:
  uint8_t m_bitMap[SIZE / 8 + 1]{};
  uint32_t m_numElements = 0;
  TYPE m_data[SIZE];
};

} // namespace rtps

#endif // RTPS_MEMORYPOOL_H

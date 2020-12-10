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

#ifndef RTPS_PBUFWRAPPER_H
#define RTPS_PBUFWRAPPER_H

#include "lwip/pbuf.h"
#include "rtps/common/types.h"

namespace rtps {

struct PBufWrapper {

  pbuf *firstElement = nullptr;

  PBufWrapper() = default;
  explicit PBufWrapper(pbuf *bufferToWrap);
  explicit PBufWrapper(DataSize_t length);

  // Shallow Copy. No copying of the underlying pbuf. Just another reference
  // like a shared pointer.
  PBufWrapper(const PBufWrapper &other);
  PBufWrapper &operator=(const PBufWrapper &other);

  PBufWrapper(PBufWrapper &&other) noexcept;
  PBufWrapper &operator=(PBufWrapper &&other) noexcept;

  ~PBufWrapper();

  PBufWrapper deepCopy() const;

  bool isValid() const;

  bool append(const uint8_t *data, DataSize_t length);

  /// Note that unused reserved memory is now part of the wrapper. New calls to
  /// append(uint8_t*[...]) will continue behind the appended wrapper
  void append(PBufWrapper &&other);

  bool reserve(DataSize_t length);

  /// After calling this function, data is added starting from the beginning
  /// again. It does not revert reserve.
  void reset();

  DataSize_t spaceLeft() const;
  DataSize_t spaceUsed() const;

private:
  constexpr static pbuf_layer m_layer = PBUF_TRANSPORT;
  constexpr static pbuf_type m_type = PBUF_POOL;

  DataSize_t m_freeSpace = 0;

  bool increaseSizeBy(uint16_t length);

  void copySimpleMembersAndResetBuffer(const PBufWrapper &other);
};

} // namespace rtps

#endif // RTPS_PBUFWRAPPER_H

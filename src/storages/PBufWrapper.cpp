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

#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/Log.h"

using rtps::PBufWrapper;

#if PBUF_WRAP_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define PBUF_WRAP_LOG(...)                                                     \
  if (true) {                                                                  \
    printf("[PBUF Wrapper] ");                                                 \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define PBUF_WRAP_LOG(...) //
#endif

PBufWrapper::PBufWrapper(pbuf *bufferToWrap) : firstElement(bufferToWrap) {
  m_freeSpace = 0; // Assume it to be full
}

PBufWrapper::PBufWrapper(DataSize_t length)
    : firstElement(pbuf_alloc(m_layer, length, m_type)) {

  if (isValid()) {
    m_freeSpace = length;
  }
}

// TODO: Uses copy assignment. Improvement possible
PBufWrapper::PBufWrapper(const PBufWrapper &other) { *this = other; }

// TODO: Uses move assignment. Improvement possible
PBufWrapper::PBufWrapper(PBufWrapper &&other) noexcept {
  *this = std::move(other);
}

PBufWrapper &PBufWrapper::operator=(const PBufWrapper &other) {
  copySimpleMembersAndResetBuffer(other);

  if (other.firstElement != nullptr) {
    pbuf_ref(other.firstElement);
  }
  firstElement = other.firstElement;
  return *this;
}

PBufWrapper &PBufWrapper::operator=(PBufWrapper &&other) noexcept {
  copySimpleMembersAndResetBuffer(other);

  if (other.firstElement != nullptr) {
    firstElement = other.firstElement;
    other.firstElement = nullptr;
  }
  return *this;
}

void PBufWrapper::copySimpleMembersAndResetBuffer(const PBufWrapper &other) {
  m_freeSpace = other.m_freeSpace;

  if (firstElement != nullptr) {
    pbuf_free(firstElement);
    firstElement = nullptr;
  }
}

PBufWrapper::~PBufWrapper() {
  if (firstElement != nullptr) {
    pbuf_free(firstElement);
  }
}

PBufWrapper PBufWrapper::deepCopy() const {
  PBufWrapper clone;
  clone.copySimpleMembersAndResetBuffer(*this);

  // Decided not to use pbuf_clone because it prevents const
  clone.firstElement = pbuf_alloc(m_layer, this->firstElement->tot_len, m_type);
  if (clone.firstElement != nullptr) {
    if (pbuf_copy(clone.firstElement, this->firstElement) != ERR_OK) {
      PBUF_WRAP_LOG("PBufWrapper::deepCopy: Copy of pbuf failed");
    }
  } else {
    clone.m_freeSpace = 0;
  }
  return clone;
}

bool PBufWrapper::isValid() const { return firstElement != nullptr; }

rtps::DataSize_t PBufWrapper::spaceLeft() const { return m_freeSpace; }

rtps::DataSize_t PBufWrapper::spaceUsed() const {
  if (firstElement == nullptr) {
    return 0;
  }

  return firstElement->tot_len - m_freeSpace;
}

bool PBufWrapper::append(const uint8_t *data, DataSize_t length) {
  if (data == nullptr) {
    return false;
  }

  err_t err = pbuf_take_at(firstElement, data, length, spaceUsed());
  if (err != ERR_OK) {
    return false;
  }

  m_freeSpace -= length;
  return true;
}

void PBufWrapper::append(PBufWrapper &&other) {
  if (this == &other) {
    return;
  }
  if (this->firstElement == nullptr) {
    *this = std::move(other);
    return;
  }

  m_freeSpace = other.m_freeSpace;
  pbuf *const newElement = other.firstElement;
  pbuf_cat(this->firstElement, newElement);

  other.firstElement = nullptr;
}

bool PBufWrapper::reserve(DataSize_t length) {
  auto additionalAllocation = length - m_freeSpace;
  if (additionalAllocation <= 0) {
    return true;
  }

  return increaseSizeBy(additionalAllocation);
}

void PBufWrapper::reset() {
  if (firstElement != nullptr) {
    m_freeSpace = firstElement->tot_len;
  }
}

bool PBufWrapper::increaseSizeBy(uint16_t length) {
  pbuf *allocation = pbuf_alloc(m_layer, length, m_type);
  if (allocation == nullptr) {
    return false;
  }

  m_freeSpace += length;

  if (firstElement == nullptr) {
    firstElement = allocation;
  } else {
    pbuf_cat(firstElement, allocation);
  }

  return true;
}

#undef PBUF_WRAP_VERBOSE

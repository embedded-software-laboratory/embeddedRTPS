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

// Copyright 2023 Apex.AI, Inc.
// All rights reserved.

#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/Log.h"

using rtps::PBufWrapper;

#if PBUF_WRAP_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef PBUF_WRAP_LOG
#include "rtps/utils/strutils.h"
#define PBUF_WRAP_LOG(...)                                                     \
  if (true) {                                                                  \
    printf("[PBUF Wrapper] ");                                                 \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#endif
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

// TODO: Uses move assignment. Improvement possible
PBufWrapper::PBufWrapper(PBufWrapper &&other) noexcept {
  *this = std::move(other);
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

void PBufWrapper::destroy()
{
  if (firstElement != nullptr) {
    pbuf_free(firstElement);
    firstElement = nullptr;
  }
  m_freeSpace = 0;
}

PBufWrapper::~PBufWrapper() {
  destroy();
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

void PBufWrapper::append(const PBufWrapper &other) {
  if (this->firstElement == nullptr) {
    m_freeSpace = other.m_freeSpace;
    this->firstElement = other.firstElement;
    pbuf_ref(this->firstElement);
    return;
  }

  m_freeSpace += other.m_freeSpace;
  pbuf_chain(this->firstElement, other.firstElement);
}

bool PBufWrapper::reserve(DataSize_t length) {
  int16_t additionalAllocation = length - m_freeSpace;
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

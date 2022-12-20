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

#ifndef RTPS_LOCK_H
#define RTPS_LOCK_H

#if defined(POSIX)
#include <mutex>
typedef std::recursive_mutex* Lock_t;
#else
#include "FreeRTOS.h"
#include "semphr.h"
typedef SemaphoreHandle_t Lock_t
#endif


namespace rtps {

class Lock {
public:
  explicit Lock(Lock_t &mutex) : m_mutex(mutex) {
#if defined(POSIX)
    m_mutex->lock();
#else
    xSemaphoreTakeRecursive(m_mutex, portMAX_DELAY);
#endif
  };

  ~Lock() {
#if defined(POSIX)
    m_mutex->unlock();
#else
    xSemaphoreGiveRecursive(m_mutex); 
#endif
    };

private:
  Lock_t m_mutex;
};

bool createMutex(Lock_t *mutex);

} // namespace rtps
#endif // RTPS_LOCK_H

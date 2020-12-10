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
<<<<<<< HEAD

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
=======
>>>>>>> fix/github-changes
*/

#ifndef RTPS_LOG_H
#define RTPS_LOG_H

#include <cstdio>
#include <stdarg.h>

#ifdef HIGHTEC_TOOLCHAIN
#include "FreeRTOS.h"
#include "TFT.h"
#include "task.h"
#endif

namespace rtps {
class Log {
public:
  Log() = delete;
  static void printLine(const char *format, ...) {
#ifdef HIGHTEC_TOOLCHAIN
    //        	constexpr uint8_t size = 25;
    //			static const uint8_t firstLine = 0;
    //			static const uint8_t lastLine = 10;
    //			static uint8_t currentLine = 0;
    //			static char output[size];
    //
    //        	taskENTER_CRITICAL();
    //          va_list argList;
    //          va_start(argList,format);
    //          vsprintf(output,format,argList);
    //          va_end(argList);
    //          TFT_PrintLine(currentLine, output);
    //          currentLine = ((currentLine + 1) % lastLine) + firstLine;
    //          taskEXIT_CRITICAL();
  }
#else
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
  }
#endif
};
} // namespace rtps

#endif // RTPS_LOG_H

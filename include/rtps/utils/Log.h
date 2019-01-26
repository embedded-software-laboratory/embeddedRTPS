/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_LOG_H
#define RTPS_LOG_H

#include <cstdio>
#include <stdarg.h>

#ifdef HIGHTEC_TOOLCHAIN
#include "TFT.h"
#include "FreeRTOS.h"
#include "task.h"
#endif

namespace rtps{
    class Log{
    public:
        Log() = delete;
        static void printLine(const char* format, ...) {
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
}

#endif //RTPS_LOG_H

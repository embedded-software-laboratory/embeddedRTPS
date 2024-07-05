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

#ifndef RTPS_LOG_H
#define RTPS_LOG_H

#include <cstdio>
#include <stdarg.h>

#define RTPS_GLOBAL_VERBOSE 0

#define SFW_VERBOSE 0
#define SPDP_VERBOSE 0
#define PBUF_WRAP_VERBOSE 0
#define SEDP_VERBOSE 0
#define RECV_VERBOSE 0
#define PARTICIPANT_VERBOSE 0
#define DOMAIN_VERBOSE 0
#define UDP_DRIVER_VERBOSE 0
#define TSCB_VERBOSE 0
#define SLW_VERBOSE 0
#define SFR_VERBOSE 0
#define SLR_VERBOSE 0
#define THREAD_POOL_VERBOSE 0

#undef SFW_LOG
#undef SPDP_LOG
#undef PBUF_LOG
#undef SEDP_LOG
#undef RECV_LOG
#undef PARTICIPANT_LOG
#undef DOMAIN_LOG
#undef UDP_DRIVER_LOG
#undef TSCB_LOG
#undef SLW_LOG
#undef SFR_LOG
#undef SLR_LOG
#undef THREAD_POOL_LOG

#endif // RTPS_LOG_H

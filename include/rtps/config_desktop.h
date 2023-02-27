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

#ifndef RTPS_CONFIG_H
#define RTPS_CONFIG_H

#include "rtps/common/types.h"

namespace rtps {

#define IS_LITTLE_ENDIAN 1

namespace Config {
const VendorId_t VENDOR_ID = {13, 37};
const std::array<uint8_t, 4> IP_ADDRESS = {
    192, 168, 1, 2}; // Needs to be set in lwipcfg.h too.
const GuidPrefix_t BASE_GUID_PREFIX{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 9};

const uint8_t DOMAIN_ID = 0; // 230 possible with UDP
const uint8_t MAX_NUM_PARTICIPANTS = 2;
const uint8_t NUM_STATELESS_WRITERS =
    MAX_NUM_PARTICIPANTS + 1; // Required + Additional
const uint8_t NUM_STATELESS_READERS =
    MAX_NUM_PARTICIPANTS + 1; // Required + Additional
const uint8_t NUM_STATEFUL_READERS =
    4; // 1-4 required per participant depending on what they do and to whom
       // they match
const uint8_t NUM_STATEFUL_WRITERS =
    4; // 1-4 required per participant depending on what they do and to whom
       // they match
const uint8_t NUM_WRITERS_PER_PARTICIPANT = 4;
const uint8_t NUM_READERS_PER_PARTICIPANT = 4;
const uint8_t NUM_WRITER_PROXIES_PER_READER = 3;
const uint8_t NUM_READER_PROXIES_PER_WRITER = 3;

const uint8_t MAX_NUM_UNMATCHED_REMOTE_WRITERS = 100;
const uint8_t MAX_NUM_UNMATCHED_REMOTE_READERS = 10;

const uint8_t HISTORY_SIZE = 10;

const uint8_t MAX_TYPENAME_LENGTH = 20;
const uint8_t MAX_TOPICNAME_LENGTH = 20;

const int HEARTBEAT_STACKSIZE = 1200;          // byte
const int THREAD_POOL_WRITER_STACKSIZE = 1100; // byte
const int THREAD_POOL_READER_STACKSIZE = 1600; // byte
const uint16_t SPDP_WRITER_STACKSIZE = 550;    // byte

const uint16_t SF_WRITER_HB_PERIOD_MS = 500;
const uint16_t SPDP_RESEND_PERIOD_MS = 10000;
const uint8_t SPDP_CYCLECOUNT_HEARTBEAT =
    2; // skip x SPDP rounds before checking liveliness
const uint8_t SPDP_WRITER_PRIO = 3;
const uint8_t SPDP_MAX_NUMBER_FOUND_PARTICIPANTS = 5;
const uint8_t SPDP_MAX_NUM_LOCATORS = 5;
const Duration_t SPDP_DEFAULT_REMOTE_LEASE_DURATION = {
    100, 0}; // Default lease duration for remote participants, usually
             // overwritten by remote info
const Duration_t SPDP_MAX_REMOTE_LEASE_DURATION = {
    180,
    0}; // Absolute maximum lease duration, ignoring remote participant info

const int MAX_NUM_UDP_CONNECTIONS = 10;

const int THREAD_POOL_NUM_WRITERS = 2;
const int THREAD_POOL_NUM_READERS = 2;
const int THREAD_POOL_WRITER_PRIO = 3;
const int THREAD_POOL_READER_PRIO = 3;
const int THREAD_POOL_WORKLOAD_QUEUE_LENGTH = 10;

constexpr int OVERALL_HEAP_SIZE =
    THREAD_POOL_NUM_WRITERS * THREAD_POOL_WRITER_STACKSIZE +
    THREAD_POOL_NUM_READERS * THREAD_POOL_READER_STACKSIZE +
    MAX_NUM_PARTICIPANTS * SPDP_WRITER_STACKSIZE +
    NUM_STATEFUL_WRITERS * HEARTBEAT_STACKSIZE;
} // namespace Config
} // namespace rtps

#endif // RTPS_CONFIG_H

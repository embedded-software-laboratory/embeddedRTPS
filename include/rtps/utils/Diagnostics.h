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

#ifndef RTPS_DIAGNOSTICS_H
#define RTPS_DIAGNOSTICS_H

#include <stdint.h>

namespace rtps {
namespace Diagnostics {

namespace ThreadPool {
extern uint32_t dropped_incoming_packets_usertraffic;
extern uint32_t dropped_incoming_packets_metatraffic;

extern uint32_t dropped_outgoing_packets_usertraffic;
extern uint32_t dropped_outgoing_packets_metatraffic;

extern uint32_t processed_incoming_metatraffic;
extern uint32_t processed_outgoing_metatraffic;
extern uint32_t processed_incoming_usertraffic;
extern uint32_t processed_outgoing_usertraffic;

extern uint32_t max_ever_elements_outgoing_usertraffic_queue;
extern uint32_t max_ever_elements_incoming_usertraffic_queue;

extern uint32_t max_ever_elements_outgoing_metatraffic_queue;
extern uint32_t max_ever_elements_incoming_metatraffic_queue;
} // namespace ThreadPool

namespace StatefulReader {
extern uint32_t sfr_unexpected_sn;
extern uint32_t sfr_retransmit_requests;
} // namespace StatefulReader

namespace Network {
extern uint32_t lwip_allocation_failures;
}

namespace OS {
extern uint32_t current_free_heap_size;
}

namespace SEDP {
extern uint32_t max_ever_remote_participants;
extern uint32_t current_remote_participants;

extern uint32_t max_ever_matched_reader_proxies;
extern uint32_t current_max_matched_reader_proxies;

extern uint32_t max_ever_matched_writer_proxies;
extern uint32_t current_max_matched_writer_proxies;

extern uint32_t max_ever_unmatched_reader_proxies;
extern uint32_t current_max_unmatched_reader_proxies;

extern uint32_t max_ever_unmatched_writer_proxies;
extern uint32_t current_max_unmatched_writer_proxies;
} // namespace SEDP

} // namespace Diagnostics
} // namespace rtps

#endif

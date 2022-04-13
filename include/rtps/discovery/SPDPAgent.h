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

#ifndef RTPS_SPDP_H
#define RTPS_SPDP_H

#include "lwip/sys.h"
#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/BuiltInEndpoints.h"
#include "rtps/discovery/ParticipantProxyData.h"
#include "ucdr/microcdr.h"

#if SPDP_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define SPDP_LOG(...)                                                          \
  if (true) {                                                                  \
    printf("[SPDP] ");                                                         \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define SPDP_LOG(...) //
#endif

namespace rtps {
class Participant;
class Writer;
class Reader;
class ReaderCacheChange;

class SPDPAgent {
public:
  ~SPDPAgent();
  void init(Participant &participant, BuiltInEndpoints &endpoints);
  void start();
  void stop();

private:
  Participant *mp_participant = nullptr;
  BuiltInEndpoints m_buildInEndpoints;
  bool m_running = false;
  std::array<uint8_t, 400> m_outputBuffer{}; // TODO check required size
  std::array<uint8_t, 400> m_inputBuffer{};
  ParticipantProxyData m_proxyDataBuffer{};
  ucdrBuffer m_microbuffer{};
  uint8_t m_cycleHB = 0;

  sys_mutex_t m_mutex;
  bool initialized = false;
  static void receiveCallback(void *callee,
                              const ReaderCacheChange &cacheChange);
  void handleSPDPPackage(const ReaderCacheChange &cacheChange);
  void configureEndianessAndOptions(ucdrBuffer &buffer);
  void processProxyData();
  bool addProxiesForBuiltInEndpoints();

  void addInlineQos();
  void addParticipantParameters();
  void endCurrentList();

  static void runBroadcast(void *args);
};
} // namespace rtps

#endif // RTPS_SPDP_H

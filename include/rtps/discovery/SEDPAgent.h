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

#ifndef RTPS_SEDPAGENT_H
#define RTPS_SEDPAGENT_H

#include "rtps/discovery/BuiltInEndpoints.h"
#include "rtps/discovery/TopicData.h"

namespace rtps {

class Participant;
class ReaderCacheChange;
class Writer;
class Reader;

class SEDPAgent {
public:
  void init(Participant &part, const BuiltInEndpoints &endpoints);
  void addWriter(Writer &writer);
  void addReader(Reader &reader);
  void registerOnNewPublisherMatchedCallback(void (*callback)(void *arg),
                                             void *args);
  void registerOnNewSubscriberMatchedCallback(void (*callback)(void *arg),
                                              void *args);
  void removeUnmatchedEntitiesOfParticipant(const GuidPrefix_t &guidPrefix);
  uint32_t getNumRemoteUnmatchedReaders();
  uint32_t getNumRemoteUnmatchedWriters();

protected: // For testing purposes
  void onNewPublisher(const TopicData &writerData);
  void onNewSubscriber(const TopicData &writerData);

private:
  Participant *m_part;
  sys_mutex_t m_mutex;
  uint8_t m_buffer[600]; // TODO check size, currently changed from 300 to 600
                         // (FastDDS gives too many options)
  BuiltInEndpoints m_endpoints;
  /*
   * If we add readers later on, remote participants will not send matching
   * writer proxies again (and vice versa). This is done only once during
   * discovery. Therefore, we need to keep track of remote endpoints. Topic and
   * type are represented as hash values to save memory.
   */
  MemoryPool<TopicDataCompressed, Config::MAX_NUM_UNMATCHED_REMOTE_WRITERS>
      m_unmatchedRemoteWriters;
  size_t m_numUnmatchedRemoteWriters = 0;
  MemoryPool<TopicDataCompressed, Config::MAX_NUM_UNMATCHED_REMOTE_READERS>
      m_unmatchedRemoteReaders;
  size_t m_numMatchedRemoteReaders = 0;

  void tryMatchUnmatchedEndpoints();
  void addUnmatchedRemoteWriter(const TopicData &writerData);
  void addUnmatchedRemoteReader(const TopicData &readerData);

  void (*mfp_onNewPublisherCallback)(void *arg) = nullptr;
  void *m_onNewPublisherArgs = nullptr;
  void (*mfp_onNewSubscriberCallback)(void *arg) = nullptr;
  void *m_onNewSubscriberArgs = nullptr;

  static void receiveCallbackPublisher(void *callee,
                                       const ReaderCacheChange &cacheChange);
  static void receiveCallbackSubscriber(void *callee,
                                        const ReaderCacheChange &cacheChange);
  void onNewPublisher(const ReaderCacheChange &change);
  void onNewSubscriber(const ReaderCacheChange &change);
};
} // namespace rtps

#endif // RTPS_SEDPAGENT_H

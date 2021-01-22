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

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/discovery/TopicData.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/Writer.h"
#include "rtps/messages/MessageTypes.h"
#include "ucdr/microcdr.h"

#define SEDP_VERBOSE 0

using rtps::SEDPAgent;

#if SEDP_VERBOSE
#ifndef STM32_PRINTF_SERIAL
uint32_t line_ = 0;
char bf_[100];

#include <asoa/driver/os.h>

#define SEDP_LOG(...)                                                          \
  if (true) {                                                                  \
    size_t t = snprintf(bf_, sizeof(bf_), __VA_ARGS__);                        \
    ASOA_ASSERT(t < sizeof(bf_), "overflow");                                  \
    TFT_PrintLine(line_ + 3, bf_);                                             \
    line_ = (line_ + 1) % 30;                                                  \
  }

#else
#define SEDP_LOG(...) printf(__VA_ARGS__)
#endif
#endif

void SEDPAgent::init(Participant &part, const BuiltInEndpoints &endpoints) {
  // TODO move
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
#if SEDP_VERBOSE
    printf("SEDPAgent failed to create mutex\n");
#endif
    return;
  }

  m_part = &part;
  m_endpoints = endpoints;
  if (m_endpoints.sedpPubReader != nullptr) {
    m_endpoints.sedpPubReader->registerCallback(receiveCallbackPublisher, this);
  }
  if (m_endpoints.sedpSubReader != nullptr) {
    m_endpoints.sedpSubReader->registerCallback(receiveCallbackSubscriber,
                                                this);
  }
}

void SEDPAgent::registerOnNewPublisherMatchedCallback(
    void (*callback)(void *arg), void *args) {
  mfp_onNewPublisherCallback = callback;
  m_onNewPublisherArgs = args;
}

void SEDPAgent::registerOnNewSubscriberMatchedCallback(
    void (*callback)(void *arg), void *args) {
  mfp_onNewSubscriberCallback = callback;
  m_onNewSubscriberArgs = args;
}

void SEDPAgent::receiveCallbackPublisher(void *callee,
                                         const ReaderCacheChange &cacheChange) {
  auto agent = static_cast<SEDPAgent *>(callee);
  agent->onNewPublisher(cacheChange);
}

void SEDPAgent::receiveCallbackSubscriber(
    void *callee, const ReaderCacheChange &cacheChange) {
  auto agent = static_cast<SEDPAgent *>(callee);
  agent->onNewSubscriber(cacheChange);
}

void SEDPAgent::onNewPublisher(const ReaderCacheChange &change) {
  Lock lock{m_mutex};
#if SEDP_VERBOSE
  SEDP_LOG("New publisher\n");
#endif

  if (!change.copyInto(m_buffer, sizeof(m_buffer) / sizeof(m_buffer[0]))) {
#if SEDP_VERBOSE
    SEDP_LOG("EDPAgent: Buffer too small.\n");
#endif
    return;
  }
  ucdrBuffer cdrBuffer;
  ucdr_init_buffer(&cdrBuffer, m_buffer, sizeof(m_buffer));

  TopicData topicData;
  if (topicData.readFromUcdrBuffer(cdrBuffer)) {
    onNewPublisher(topicData);
  }
}

void SEDPAgent::onNewPublisher(const TopicData &writerData) {
  // TODO Is it okay to add Endpoint if the respective participant is unknown
  // participant?
  if (!m_part->findRemoteParticipant(writerData.endpointGuid.prefix)) {
    return;
  }
#if SEDP_VERBOSE
  SEDP_LOG("PUB T/D %s/%s\n", writerData.topicName, writerData.typeName);
#endif
  Reader *reader = m_part->getMatchingReader(writerData);
  if (reader == nullptr) {
#if SEDP_VERBOSE
    SEDP_LOG("SEDPAgent: Couldn't find reader for new Publisher[%s, %s]\n",
             writerData.topicName, writerData.typeName);
#endif
    return;
  }
  // TODO check policies
#if SEDP_VERBOSE
  SEDP_LOG("Found a new ");
  if (writerData.reliabilityKind == ReliabilityKind_t::RELIABLE) {
    SEDP_LOG("reliable ");
  } else {
    SEDP_LOG("best-effort ");
  }
  SEDP_LOG("publisher\n");
#endif
  reader->addNewMatchedWriter(
      WriterProxy{writerData.endpointGuid, writerData.unicastLocator});
  if (mfp_onNewPublisherCallback != nullptr) {
    mfp_onNewPublisherCallback(m_onNewPublisherArgs);
  }
}

void SEDPAgent::onNewSubscriber(const ReaderCacheChange &change) {
  Lock lock{m_mutex};
#if SEDP_VERBOSE
  SEDP_LOG("New subscriber\n");
#endif

  if (!change.copyInto(m_buffer, sizeof(m_buffer) / sizeof(m_buffer[0]))) {
#if SEDP_VERBOSE
    SEDP_LOG("SEDPAgent: Buffer too small.");
#endif
    return;
  }
  ucdrBuffer cdrBuffer;
  ucdr_init_buffer(&cdrBuffer, m_buffer, sizeof(m_buffer));

  TopicData topicData;
  if (topicData.readFromUcdrBuffer(cdrBuffer)) {
    onNewSubscriber(topicData);
  }
}

void SEDPAgent::onNewSubscriber(const TopicData &readerData) {
  if (!m_part->findRemoteParticipant(readerData.endpointGuid.prefix)) {
    return;
  }
  Writer *writer = m_part->getMatchingWriter(readerData);
#if SEDP_VERBOSE
  SEDP_LOG("SUB T/D %s/%s\n", readerData.topicName, readerData.typeName);
#endif
  if (writer == nullptr) {
#if SEDP_VERBOSE
    SEDP_LOG("SEDPAgent: Couldn't find writer for new subscriber[%s, %s]\n",
             readerData.topicName, readerData.typeName);
#endif
    return;
  }

  // TODO check policies
#if SEDP_VERBOSE
  SEDP_LOG("Found a new ");
  if (readerData.reliabilityKind == ReliabilityKind_t::RELIABLE) {
    SEDP_LOG("reliable ");
  } else {
    SEDP_LOG("best-effort ");
  }
  SEDP_LOG("Subscriber\n");
#endif
  writer->addNewMatchedReader(
      ReaderProxy{readerData.endpointGuid, readerData.unicastLocator});
  if (mfp_onNewSubscriberCallback != nullptr) {
    mfp_onNewSubscriberCallback(m_onNewSubscriberArgs);
  }
}

void SEDPAgent::addWriter(Writer &writer) {
  if (m_endpoints.sedpPubWriter == nullptr) {
    return;
  }
  EntityKind_t writerKind =
      writer.m_attributes.endpointGuid.entityId.entityKind;
  if (writerKind == EntityKind_t::BUILD_IN_WRITER_WITH_KEY ||
      writerKind == EntityKind_t::BUILD_IN_WRITER_WITHOUT_KEY) {
    return; // No need to announce builtin endpoints
  }

  Lock lock{m_mutex};
  ucdrBuffer microbuffer;
  ucdr_init_buffer(&microbuffer, m_buffer,
                   sizeof(m_buffer) / sizeof(m_buffer[0]));
  const uint16_t zero_options = 0;

  ucdr_serialize_array_uint8_t(&microbuffer,
                               rtps::SMElement::SCHEME_PL_CDR_LE.data(),
                               rtps::SMElement::SCHEME_PL_CDR_LE.size());
  ucdr_serialize_uint16_t(&microbuffer, zero_options);
  writer.m_attributes.serializeIntoUcdrBuffer(microbuffer);
  m_endpoints.sedpPubWriter->newChange(ChangeKind_t::ALIVE, m_buffer,
                                       ucdr_buffer_length(&microbuffer));
#if SEDP_VERBOSE
  SEDP_LOG("Added new change to sedpPubWriter.\n");
#endif
}

void SEDPAgent::addReader(Reader &reader) {
  if (m_endpoints.sedpSubWriter == nullptr) {
    return;
  }

  EntityKind_t readerKind =
      reader.m_attributes.endpointGuid.entityId.entityKind;
  if (readerKind == EntityKind_t::BUILD_IN_READER_WITH_KEY ||
      readerKind == EntityKind_t::BUILD_IN_READER_WITHOUT_KEY) {
    return; // No need to announce builtin endpoints
  }

  Lock lock{m_mutex};
  ucdrBuffer microbuffer;
  ucdr_init_buffer(&microbuffer, m_buffer,
                   sizeof(m_buffer) / sizeof(m_buffer[0]));
  const uint16_t zero_options = 0;

  ucdr_serialize_array_uint8_t(&microbuffer,
                               rtps::SMElement::SCHEME_PL_CDR_LE.data(),
                               rtps::SMElement::SCHEME_PL_CDR_LE.size());
  ucdr_serialize_uint16_t(&microbuffer, zero_options);
  reader.m_attributes.serializeIntoUcdrBuffer(microbuffer);
  m_endpoints.sedpSubWriter->newChange(ChangeKind_t::ALIVE, m_buffer,
                                       ucdr_buffer_length(&microbuffer));

#if SEDP_VERBOSE
  SEDP_LOG("Added new change to sedpSubWriter.\n");
#endif
}

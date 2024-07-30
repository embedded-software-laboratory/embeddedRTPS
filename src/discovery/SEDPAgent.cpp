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
#include "rtps/utils/Log.h"
#include "ucdr/microcdr.h"

using rtps::SEDPAgent;

#if SEDP_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef SEDP_LOG
#define SEDP_LOG(...)                                                          \
  if (true) {                                                                  \
    printf("[SEDP] ");                                                         \
    printf(__VA_ARGS__);                                                       \
    printf("\r\n");                                                            \
  }
#endif
#else
#define SEDP_LOG(...) //
#endif

void SEDPAgent::init(Participant &part, const BuiltInEndpoints &endpoints) {
  // TODO move
  if (!createMutex(&m_mutex)) {
    SEDP_LOG("SEDPAgent failed to create mutex\n");
    return;
  }

  m_part = &part;
  m_endpoints = endpoints;
  if (m_endpoints.sedpPubReader != nullptr) {
    m_endpoints.sedpPubReader->registerCallback(jumppadPublisherReader, this);
  }
  if (m_endpoints.sedpSubReader != nullptr) {
    m_endpoints.sedpSubReader->registerCallback(jumppadSubscriptionReader,
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

void SEDPAgent::jumppadPublisherReader(void *callee,
                                       const ReaderCacheChange &cacheChange) {
  auto agent = static_cast<SEDPAgent *>(callee);
  agent->handlePublisherReaderMessage(cacheChange);
}

void SEDPAgent::jumppadSubscriptionReader(
    void *callee, const ReaderCacheChange &cacheChange) {
  auto agent = static_cast<SEDPAgent *>(callee);
  agent->handleSubscriptionReaderMessage(cacheChange);
}

void SEDPAgent::handlePublisherReaderMessage(const ReaderCacheChange &change) {
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
    handlePublisherReaderMessage(topicData, change);
  }
}

void SEDPAgent::addUnmatchedRemoteWriter(const TopicData &writerData) {
  addUnmatchedRemoteWriter(TopicDataCompressed(writerData));
}

void SEDPAgent::addUnmatchedRemoteReader(const TopicData &readerData) {
  addUnmatchedRemoteReader(TopicDataCompressed(readerData));
}

void SEDPAgent::addUnmatchedRemoteWriter(
    const TopicDataCompressed &writerData) {
  if (m_unmatchedRemoteWriters.isFull()) {
#if SEDP_VERBOSE
    SEDP_LOG("List of unmatched remote writers is full.\n");
#endif
    return;
  }
  SEDP_LOG("Adding unmatched remote writer %zx %zx.\n", writerData.topicHash,
           writerData.typeHash);
  m_unmatchedRemoteWriters.add(writerData);
}

void SEDPAgent::addUnmatchedRemoteReader(
    const TopicDataCompressed &readerData) {
  if (m_unmatchedRemoteReaders.isFull()) {
#if SEDP_VERBOSE
    SEDP_LOG("List of unmatched remote readers is full.\n");
#endif
    return;
  }
  SEDP_LOG("Adding unmatched remote reader %zx %zx.\n", readerData.topicHash,
           readerData.typeHash);
  m_unmatchedRemoteReaders.add(readerData);
}

void SEDPAgent::removeUnmatchedEntity(const Guid_t &guid) {
  auto isElementToRemove = [&](const TopicDataCompressed &topicData) {
    return topicData.endpointGuid == guid;
  };

  auto thunk = [](void *arg, const TopicDataCompressed &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_unmatchedRemoteReaders.remove(thunk, &isElementToRemove);
  m_unmatchedRemoteWriters.remove(thunk, &isElementToRemove);
}

void SEDPAgent::removeUnmatchedEntitiesOfParticipant(
    const GuidPrefix_t &guidPrefix) {
  Lock lock{m_mutex};
  auto isElementToRemove = [&](const TopicDataCompressed &topicData) {
    return topicData.endpointGuid.prefix == guidPrefix;
  };

  auto thunk = [](void *arg, const TopicDataCompressed &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_unmatchedRemoteReaders.remove(thunk, &isElementToRemove);
  m_unmatchedRemoteWriters.remove(thunk, &isElementToRemove);
}

uint32_t SEDPAgent::getNumRemoteUnmatchedReaders() {
  return m_unmatchedRemoteReaders.getNumElements();
}

uint32_t SEDPAgent::getNumRemoteUnmatchedWriters() {
  return m_unmatchedRemoteWriters.getNumElements();
}

void SEDPAgent::handlePublisherReaderMessage(const TopicData &writerData,
                                             const ReaderCacheChange &change) {
  // TODO Is it okay to add Endpoint if the respective participant is unknown
  // participant?
  if (!m_part->findRemoteParticipant(writerData.endpointGuid.prefix)) {
    return;
  }

  if (writerData.isDisposedFlagSet() || writerData.isUnregisteredFlagSet()) {
    handleRemoteEndpointDeletion(writerData, change);
    return;
  }

#if SEDP_VERBOSE
  SEDP_LOG("PUB T/D %s/%s", writerData.topicName, writerData.typeName);
#endif
  Reader *reader = m_part->getMatchingReader(writerData);
  if (reader == nullptr) {
#if SEDP_VERBOSE
    SEDP_LOG("SEDPAgent: Couldn't find reader for new Publisher[%s, %s] \n",
             writerData.topicName, writerData.typeName);
#endif
    addUnmatchedRemoteWriter(writerData);
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
      WriterProxy{writerData.endpointGuid, writerData.unicastLocator,
                  (writerData.reliabilityKind == ReliabilityKind_t::RELIABLE)});
  if (mfp_onNewPublisherCallback != nullptr) {
    mfp_onNewPublisherCallback(m_onNewPublisherArgs);
  }
}

void SEDPAgent::handleSubscriptionReaderMessage(
    const ReaderCacheChange &change) {
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
    handleSubscriptionReaderMessage(topicData, change);
  }
}

void SEDPAgent::handleRemoteEndpointDeletion(const TopicData &topic,
                                             const ReaderCacheChange &change) {
  SEDP_LOG("Endpoint deletion message SN %u.%u GUID %u %u %u %u \r\n",
           (int)change.sn.high, (int)change.sn.low,
           change.writerGuid.prefix.id[0], change.writerGuid.prefix.id[1],
           change.writerGuid.prefix.id[2], change.writerGuid.prefix.id[3]);
  if (!topic.entityIdFromKeyHashValid) {
    return;
  }

  Guid_t guid;
  guid.prefix = topic.endpointGuid.prefix;
  guid.entityId = topic.entityIdFromKeyHash;

  // Remove entity ID from all proxies of local endpoints
  m_part->removeProxyFromAllEndpoints(guid);

  // Remove entity ID from unmatched endpoints
  removeUnmatchedEntity(guid);
}

void SEDPAgent::handleSubscriptionReaderMessage(
    const TopicData &readerData, const ReaderCacheChange &change) {
  if (!m_part->findRemoteParticipant(readerData.endpointGuid.prefix)) {
    return;
  }

  if (readerData.isDisposedFlagSet() || readerData.isUnregisteredFlagSet()) {
    handleRemoteEndpointDeletion(readerData, change);
    return;
  }

  Writer *writer = m_part->getMatchingWriter(readerData);
#if SEDP_VERBOSE
  SEDP_LOG("SUB T/D %s/%s", readerData.topicName, readerData.typeName);
#endif
  if (writer == nullptr) {
#if SEDP_VERBOSE
    SEDP_LOG("SEDPAgent: Couldn't find writer for new subscriber[%s, %s]\n",
             readerData.topicName, readerData.typeName);
#endif
    addUnmatchedRemoteReader(readerData);
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
  if (readerData.multicastLocator.kind ==
      rtps::LocatorKind_t::LOCATOR_KIND_UDPv4) {
    writer->addNewMatchedReader(ReaderProxy{
        readerData.endpointGuid, readerData.unicastLocator,
        readerData.multicastLocator,
        (readerData.reliabilityKind == ReliabilityKind_t::RELIABLE)});
  } else {
    writer->addNewMatchedReader(ReaderProxy{
        readerData.endpointGuid, readerData.unicastLocator,
        (readerData.reliabilityKind == ReliabilityKind_t::RELIABLE)});
  }

  if (mfp_onNewSubscriberCallback != nullptr) {
    mfp_onNewSubscriberCallback(m_onNewSubscriberArgs);
  }
}

void SEDPAgent::tryMatchUnmatchedEndpoints() {
  // Try to match remote readers with local writers
  for (auto &proxy : m_unmatchedRemoteReaders) {
    auto writer = m_part->getMatchingWriter(proxy);
    if (writer != nullptr) {
      writer->addNewMatchedReader(
          ReaderProxy{proxy.endpointGuid, proxy.unicastLocator,
                      proxy.multicastLocator, proxy.is_reliable});
      removeUnmatchedEntity(proxy.endpointGuid);
    }
  }

  // Try to match remote writers with local readers
  for (auto &proxy : m_unmatchedRemoteWriters) {
    auto reader = m_part->getMatchingReader(proxy);
    if (reader != nullptr) {
      reader->addNewMatchedWriter(WriterProxy{
          proxy.endpointGuid, proxy.unicastLocator, proxy.is_reliable});
      removeUnmatchedEntity(proxy.endpointGuid);
    }
  }
}

bool SEDPAgent::addWriter(Writer &writer) {
  if (m_endpoints.sedpPubWriter == nullptr) {
    return true;
  }
  EntityKind_t writerKind =
      writer.m_attributes.endpointGuid.entityId.entityKind;
  if (writerKind == EntityKind_t::BUILD_IN_WRITER_WITH_KEY ||
      writerKind == EntityKind_t::BUILD_IN_WRITER_WITHOUT_KEY) {
    return true; // No need to announce builtin endpoints
  }

  Lock lock{m_mutex};

  // Check unmatched writers for this new reader
  tryMatchUnmatchedEndpoints();

  ucdrBuffer microbuffer;
  ucdr_init_buffer(&microbuffer, m_buffer,
                   sizeof(m_buffer) / sizeof(m_buffer[0]));
  const uint16_t zero_options = 0;

  ucdr_serialize_array_uint8_t(&microbuffer,
                               rtps::SMElement::SCHEME_PL_CDR_LE.data(),
                               rtps::SMElement::SCHEME_PL_CDR_LE.size());
  ucdr_serialize_uint16_t(&microbuffer, zero_options);
  writer.m_attributes.serializeIntoUcdrBuffer(microbuffer);
  auto change = m_endpoints.sedpPubWriter->newChange(
      ChangeKind_t::ALIVE, m_buffer, ucdr_buffer_length(&microbuffer));
  writer.setSEDPSequenceNumber(change->sequenceNumber);
  return (change != nullptr);
#if SEDP_VERBOSE
  SEDP_LOG("Added new change to sedpPubWriter.\n");
#endif
}

template <typename A>
bool SEDPAgent::disposeEndpointInSEDPHistory(A *local_endpoint,
                                             Writer *sedp_writer) {
  return sedp_writer->removeFromHistory(
      local_endpoint->getSEDPSequenceNumber());
}

template <typename A>
bool SEDPAgent::announceEndpointDeletion(A *local_endpoint,
                                         Writer *sedp_endpoint) {
  ucdrBuffer microbuffer;
  ucdr_init_buffer(&microbuffer, m_buffer,
                   sizeof(m_buffer) / sizeof(m_buffer[0]));

  ucdr_serialize_uint16_t(&microbuffer, ParameterId::PID_KEY_HASH);
  ucdr_serialize_uint16_t(&microbuffer, 16);
  ucdr_serialize_array_uint8_t(
      &microbuffer, local_endpoint->m_attributes.endpointGuid.prefix.id.data(),
      sizeof(GuidPrefix_t::id));
  ucdr_serialize_array_uint8_t(&microbuffer, local_endpoint->m_attributes.endpointGuid.entityId.entityKey.data(), 3);
  ucdr_serialize_uint8_t(&microbuffer, static_cast<uint8_t>(local_endpoint->m_attributes.endpointGuid.entityId.entityKind));

  ucdr_serialize_uint16_t(&microbuffer, ParameterId::PID_STATUS_INFO);
  ucdr_serialize_uint16_t(&microbuffer, static_cast<uint16_t>(4));
  ucdr_serialize_uint8_t(&microbuffer, 0);
  ucdr_serialize_uint8_t(&microbuffer, 0);
  ucdr_serialize_uint8_t(&microbuffer, 0);
  ucdr_serialize_uint8_t(&microbuffer, 3);

  // Sentinel to terminate inline qos
  ucdr_serialize_uint16_t(&microbuffer, ParameterId::PID_SENTINEL);
  ucdr_serialize_uint16_t(&microbuffer, 0);

  // Sentinel to terminate serialized data
  ucdr_serialize_uint16_t(&microbuffer, ParameterId::PID_SENTINEL);
  ucdr_serialize_uint16_t(&microbuffer, 0);

  auto ret =
      sedp_endpoint->newChange(ChangeKind_t::ALIVE, m_buffer,
                               ucdr_buffer_length(&microbuffer), true, true);
  SEDP_LOG("Annoucing endpoint delete, SN = %u.%u\r\n",
           (int)ret->sequenceNumber.low, (int)ret->sequenceNumber.high);
  return (ret != nullptr);
}

void SEDPAgent::jumppadTakeProxyOfDisposedReader(const Reader *reader,
                                                 const WriterProxy &proxy,
                                                 void *arg) {
  auto agent = static_cast<SEDPAgent *>(arg);
  TopicDataCompressed topic_data(reader->m_attributes);
  topic_data.endpointGuid = proxy.remoteWriterGuid;
  topic_data.is_reliable = proxy.is_reliable;
  topic_data.multicastLocator.kind = LocatorKind_t::LOCATOR_KIND_INVALID;
  topic_data.unicastLocator = proxy.remoteLocator;
  agent->addUnmatchedRemoteWriter(topic_data);
}

void SEDPAgent::jumppadTakeProxyOfDisposedWriter(const Writer *writer,
                                                 const ReaderProxy &proxy,
                                                 void *arg) {
  auto agent = static_cast<SEDPAgent *>(arg);
  TopicDataCompressed topic_data(writer->m_attributes);
  topic_data.endpointGuid = proxy.remoteReaderGuid;
  topic_data.is_reliable = proxy.is_reliable;
  topic_data.multicastLocator.kind = LocatorKind_t::LOCATOR_KIND_INVALID;
  topic_data.unicastLocator = proxy.remoteLocator;
  agent->addUnmatchedRemoteReader(topic_data);
}

bool SEDPAgent::deleteReader(Reader *reader) {
  Lock lock{m_mutex};
  // Set cache change kind in SEDP endpoint to DISPOSED
  if (!disposeEndpointInSEDPHistory(reader, m_endpoints.sedpSubWriter)) {
    return false;
  }

  // Create Deletion Message [UD] and add to corret builtin endpoint
  if (!announceEndpointDeletion(reader, m_endpoints.sedpSubWriter)) {
    return false;
  }

  // Move all matched proxies of this endpoint to the list of unmatched
  // endpoints
  reader->dumpAllProxies(SEDPAgent::jumppadTakeProxyOfDisposedReader, this);

  return true;
}

bool SEDPAgent::deleteWriter(Writer *writer) {
  Lock lock{m_mutex};
  // Set cache change kind in SEDP endpoint to DISPOSED
  if (!disposeEndpointInSEDPHistory(writer, m_endpoints.sedpPubWriter)) {
    return false;
  }

  // Create Deletion Mesasge [UD] and add to corret builtin endpoint
  if (!announceEndpointDeletion(writer, m_endpoints.sedpPubWriter)) {
    return false;
  }

  // Move all matched proxies of this endpoint to the list of unmatched
  // endpoints
  writer->dumpAllProxies(SEDPAgent::jumppadTakeProxyOfDisposedWriter, this);

  return true;
}

bool SEDPAgent::addReader(Reader &reader) {
  if (m_endpoints.sedpSubWriter == nullptr) {
    return true;
  }

  EntityKind_t readerKind =
      reader.m_attributes.endpointGuid.entityId.entityKind;
  if (readerKind == EntityKind_t::BUILD_IN_READER_WITH_KEY ||
      readerKind == EntityKind_t::BUILD_IN_READER_WITHOUT_KEY) {
    return true; // No need to announce builtin endpoints
  }

  Lock lock{m_mutex};

  // Check unmatched writers for this new reader
  tryMatchUnmatchedEndpoints();

  ucdrBuffer microbuffer;
  ucdr_init_buffer(&microbuffer, m_buffer,
                   sizeof(m_buffer) / sizeof(m_buffer[0]));
  const uint16_t zero_options = 0;

  ucdr_serialize_array_uint8_t(&microbuffer,
                               rtps::SMElement::SCHEME_PL_CDR_LE.data(),
                               rtps::SMElement::SCHEME_PL_CDR_LE.size());
  ucdr_serialize_uint16_t(&microbuffer, zero_options);
  reader.m_attributes.serializeIntoUcdrBuffer(microbuffer);
  auto change = m_endpoints.sedpSubWriter->newChange(
      ChangeKind_t::ALIVE, m_buffer, ucdr_buffer_length(&microbuffer));
  reader.setSEDPSequenceNumber(change->sequenceNumber);
  return (change != nullptr);
#if SEDP_VERBOSE
  SEDP_LOG("Added new change to sedpSubWriter.\n");
#endif
}

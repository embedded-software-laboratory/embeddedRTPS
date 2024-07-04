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

#include "rtps/entities/Participant.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/Writer.h"
#include "rtps/messages/MessageReceiver.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#if PARTICIPANT_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef PARTICIPANT_LOG
#define PARTICIPANT_LOG(...)                                                   \
  if (true) {                                                                  \
    printf("[Participant] ");                                                  \
    printf(__VA_ARGS__);                                                       \
    printf("\r\n");                                                            \
  }
#endif
#else
#define PARTICIPANT_LOG(...) //
#endif

using rtps::Participant;

Participant::Participant()
    : m_guidPrefix(GUIDPREFIX_UNKNOWN), m_participantId(PARTICIPANT_ID_INVALID),
      m_receiver(this) {
  if (!createMutex(&m_mutex)) {
    std::terminate();
  }
}
Participant::Participant(const GuidPrefix_t &guidPrefix,
                         ParticipantId_t participantId)
    : m_guidPrefix(guidPrefix), m_participantId(participantId),
      m_receiver(this) {
  if (!createMutex(&m_mutex)) {
    while (1)
      ;
  }
}

Participant::~Participant() { m_spdpAgent.stop(); }

void Participant::reuse(const GuidPrefix_t &guidPrefix,
                        ParticipantId_t participantId) {
  m_guidPrefix = guidPrefix;
  m_participantId = participantId;
}

bool Participant::isValid() {
  return m_participantId != PARTICIPANT_ID_INVALID;
}

std::array<uint8_t, 3> Participant::getNextUserEntityKey() {
  const auto result = m_nextUserEntityId;

  ++m_nextUserEntityId[2];
  if (m_nextUserEntityId[2] == 0) {
    ++m_nextUserEntityId[1];
    if (m_nextUserEntityId[1] == 0) {
      ++m_nextUserEntityId[0];
    }
  }
  return result;
}

bool Participant::registerOnNewPublisherMatchedCallback(
    void (*callback)(void *arg), void *args) {
  if (!m_hasBuilInEndpoints) {
    return false;
  }

  m_sedpAgent.registerOnNewPublisherMatchedCallback(callback, args);
  return true;
}

bool Participant::registerOnNewSubscriberMatchedCallback(
    void (*callback)(void *arg), void *args) {
  if (!m_hasBuilInEndpoints) {
    return false;
  }

  m_sedpAgent.registerOnNewSubscriberMatchedCallback(callback, args);
  return true;
}

rtps::Writer *Participant::addWriter(Writer *pWriter) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_writers.size(); i++) {
    if (m_writers[i] == nullptr) {
      m_writers[i] = pWriter;
      if (m_hasBuilInEndpoints) {
        m_sedpAgent.addWriter(*pWriter);
      }
      return pWriter;
    }
  }
  return nullptr;
}

bool Participant::isWritersFull() {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_writers.size(); i++) {
    if (m_writers[i] == nullptr) {
      return false;
    }
  }

  return true;
}

rtps::Reader *Participant::addReader(Reader *pReader) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] == nullptr) {
      m_readers[i] = pReader;
      if (m_hasBuilInEndpoints) {
        m_sedpAgent.addReader(*pReader);
      }
      return pReader;
    }
  }

  return nullptr;
}

bool Participant::deleteReader(Reader *reader) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i]->getSEDPSequenceNumber() ==
        reader->getSEDPSequenceNumber()) {
      if (m_sedpAgent.deleteReader(reader)) {
        m_readers[i] = nullptr;
        return true;
      }
      PARTICIPANT_LOG("Found reader but SEDP deletion failed");
    }
  }
  return false;
}

bool Participant::deleteWriter(Writer *writer) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_writers.size(); i++) {
    if (m_writers[i]->getSEDPSequenceNumber() ==
        writer->getSEDPSequenceNumber()) {
      if (m_sedpAgent.deleteWriter(writer)) {
        m_writers[i] = nullptr;
        return true;
      }
      PARTICIPANT_LOG("Found reader but SEDP deletion failed");
    }
  }
  return false;
}

bool Participant::isReadersFull() {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] == nullptr) {
      return false;
    }
  }

  return true;
}

rtps::Writer *Participant::getWriter(EntityId_t id) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_writers.size(); ++i) {
    if (m_writers[i] == nullptr) {
      continue;
    }
    if (m_writers[i]->m_attributes.endpointGuid.entityId == id) {
      return m_writers[i];
    }
  }
  return nullptr;
}

rtps::Reader *Participant::getReader(EntityId_t id) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_readers.size(); ++i) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (m_readers[i]->m_attributes.endpointGuid.entityId == id) {
      return m_readers[i];
    }
  }
  return nullptr;
}

rtps::Reader *Participant::getReaderByWriterId(const Guid_t &guid) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_readers.size(); ++i) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (m_readers[i]->isProxy(guid)) {
      return m_readers[i];
    }
  }
  return nullptr;
}

rtps::Writer *Participant::getMatchingWriter(const TopicData &readerTopicData) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_writers.size(); ++i) {
    if (m_writers[i] == nullptr) {
      continue;
    }
    if (m_writers[i]->m_attributes.matchesTopicOf(readerTopicData) &&
        (readerTopicData.reliabilityKind == ReliabilityKind_t::BEST_EFFORT ||
         m_writers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::RELIABLE)) {
      return m_writers[i];
    }
  }
  return nullptr;
}

rtps::Reader *Participant::getMatchingReader(const TopicData &writerTopicData) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_readers.size(); ++i) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (m_readers[i]->m_attributes.matchesTopicOf(writerTopicData) &&
        (writerTopicData.reliabilityKind == ReliabilityKind_t::RELIABLE ||
         m_readers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::BEST_EFFORT)) {
      return m_readers[i];
    }
  }
  return nullptr;
}

rtps::Writer *
Participant::getMatchingWriter(const TopicDataCompressed &readerTopicData) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_writers.size(); ++i) {
    if (m_writers[i] == nullptr) {
      continue;
    }
    if (readerTopicData.matchesTopicOf(m_writers[i]->m_attributes) &&
        (readerTopicData.is_reliable == false ||
         m_writers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::RELIABLE)) {
      return m_writers[i];
    }
  }
  return nullptr;
}

rtps::Reader *
Participant::getMatchingReader(const TopicDataCompressed &writerTopicData) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_readers.size(); ++i) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (writerTopicData.matchesTopicOf(m_readers[i]->m_attributes) &&
        (writerTopicData.is_reliable == true ||
         m_readers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::BEST_EFFORT)) {
      return m_readers[i];
    }
  }
  return nullptr;
}

bool Participant::addNewRemoteParticipant(
    const ParticipantProxyData &remotePart) {
  Lock lock{m_mutex};
  return m_remoteParticipants.add(remotePart);
}

bool Participant::removeRemoteParticipant(const GuidPrefix_t &prefix) {
  Lock lock{m_mutex};
  auto isElementToRemove = [&](const ParticipantProxyData &proxy) {
    return proxy.m_guid.prefix == prefix;
  };
  auto thunk = [](void *arg, const ParticipantProxyData &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };
  removeAllProxiesOfParticipant(prefix);
  m_sedpAgent.removeUnmatchedEntitiesOfParticipant(prefix);
  return m_remoteParticipants.remove(thunk, &isElementToRemove);
}

void Participant::removeAllProxiesOfParticipant(const GuidPrefix_t &prefix) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    m_readers[i]->removeAllProxiesOfParticipant(prefix);
  }

  for (unsigned int i = 0; i < m_writers.size(); i++) {
    if (m_writers[i] == nullptr) {
      continue;
    }
    m_writers[i]->removeAllProxiesOfParticipant(prefix);
  }
}

void Participant::removeProxyFromAllEndpoints(const Guid_t &guid) {
  Lock lock{m_mutex};
  for (unsigned int i = 0; i < m_writers.size(); i++) {
    if (m_writers[i] == nullptr) {
      continue;
    }
    if (m_writers[i]->removeProxy(guid)) {
      PARTICIPANT_LOG("Removing proxy for writer [%s, %s], proxies left = %u\n",
                      m_writers[i]->m_attributes.topicName,
                      m_writers[i]->m_attributes.typeName,
                      (int)m_writers[i]->getProxiesCount());
    }
  }

  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (m_readers[i]->removeProxy(guid)) {
      PARTICIPANT_LOG("Removing proxy for reader [%s, %s], proxies left = %u\n",
                      m_readers[i]->m_attributes.topicName,
                      m_readers[i]->m_attributes.typeName,
                      (int)m_readers[i]->getProxiesCount());
    }
  }
}

const rtps::ParticipantProxyData *
Participant::findRemoteParticipant(const GuidPrefix_t &prefix) {
  Lock lock{m_mutex};
  auto isElementToFind = [&](const ParticipantProxyData &proxy) {
    return proxy.m_guid.prefix == prefix;
  };
  auto thunk = [](void *arg, const ParticipantProxyData &value) {
    return (*static_cast<decltype(isElementToFind) *>(arg))(value);
  };
  return m_remoteParticipants.find(thunk, &isElementToFind);
}

void Participant::refreshRemoteParticipantLiveliness(
    const GuidPrefix_t &prefix) {
  Lock lock{m_mutex};
  auto isElementToFind = [&](const ParticipantProxyData &proxy) {
    return proxy.m_guid.prefix == prefix;
  };
  auto thunk = [](void *arg, const ParticipantProxyData &value) {
    return (*static_cast<decltype(isElementToFind) *>(arg))(value);
  };

  auto remoteParticipant = m_remoteParticipants.find(thunk, &isElementToFind);
  if (remoteParticipant != nullptr) {
    remoteParticipant->onAliveSignal();
  }
}

bool Participant::hasReaderWithMulticastLocator(ip4_addr_t address) {
  Lock lock{m_mutex};
  for (uint8_t i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] == nullptr) {
      continue;
    }
    if (m_readers[i]->m_attributes.multicastLocator.isSameAddress(&address)) {
      return true;
    }
  }
  return false;
}

uint32_t Participant::getRemoteParticipantCount() {
  Lock lock{m_mutex};
  return m_remoteParticipants.getNumElements();
}

rtps::MessageReceiver *Participant::getMessageReceiver() { return &m_receiver; }

bool Participant::checkAndResetHeartbeats() {
  Lock lock1{m_mutex};
  Lock lock2{m_spdpAgent.m_mutex};
  PARTICIPANT_LOG("Have %u remote participants",
                  (unsigned int)m_remoteParticipants.getNumElements());
  PARTICIPANT_LOG(
      "Unmatched remote writers/readers, %u / %u",
      static_cast<unsigned int>(m_sedpAgent.getNumRemoteUnmatchedWriters()),
      static_cast<unsigned int>(m_sedpAgent.getNumRemoteUnmatchedReaders()));
  for (auto &remote : m_remoteParticipants) {
    PARTICIPANT_LOG("Remote GUID = %u %u %u %u | Age = %u [ms]",
                    remote.m_guid.prefix.id[4], remote.m_guid.prefix.id[5], remote.m_guid.prefix.id[6], remote.m_guid.prefix.id[7], (unsigned int)remote.getAliveSignalAgeInMilliseconds() );
    if (remote.isAlive()) {
      continue;
    }
    PARTICIPANT_LOG("removing remote participant");
    bool success = removeRemoteParticipant(remote.m_guid.prefix);
    if (!success) {
      return false;
    }else{
    	return true;
    }
  }
  return true;
}

void Participant::printInfo() {

  uint32_t max_reader_proxies = 0;
  for (unsigned int i = 0; i < m_readers.size(); i++) {
    if (m_readers[i] != nullptr && m_readers[i]->isInitialized()) {
      if (m_hasBuilInEndpoints && i < 3) {
#ifdef PARTICIPANT_PRINTINFO_LONG
        if (m_readers[i]->m_attributes.endpointGuid.entityId ==
            ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER) {
          printf("Reader %u: SPDP BUILTIN READER | Remote Proxies = %u \r\n ",
                 i, static_cast<int>(m_readers[i]->getProxiesCount()));
        }
        if (m_readers[i]->m_attributes.endpointGuid.entityId ==
            ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER) {
          printf(
              "Reader %u: SEDP PUBLICATION READER | Remote Proxies = %u \r\n ",
              i, static_cast<int>(m_readers[i]->getProxiesCount()));
        }
        if (m_readers[i]->m_attributes.endpointGuid.entityId ==
            ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER) {
          printf(
              "Reader %u: SEDP SUBSCRIPTION READER | Remote Proxies = %u \r\n",
              i, static_cast<int>(m_readers[i]->getProxiesCount()));
        }
#endif
        continue;
      }

      max_reader_proxies =
          std::max(max_reader_proxies, m_readers[i]->getProxiesCount());
#ifdef PARTICIPANT_PRINTINFO_LONG
      printf("Reader %u: Topic = %s | Type = %s | Remote Proxies = %u | SEDP "
             "SN = %u   \r\n ",
             i, m_readers[i]->m_attributes.topicName,
             m_readers[i]->m_attributes.typeName,
             static_cast<int>(m_readers[i]->getProxiesCount()),
             static_cast<int>(m_readers[i]->getSEDPSequenceNumber().low));
#endif
    }
  }

  uint32_t max_writer_proxies = 0;
  for (unsigned int i = 0; i < m_writers.size(); i++) {

    if (m_hasBuilInEndpoints && i < 3) {
#ifdef PARTICIPANT_PRINTINFO_LONG
      if (m_writers[i]->m_attributes.endpointGuid.entityId ==
          ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER) {
        printf("Writer %u: SPDP WRITER | Remote Proxies = %u  \r\n  ", i,
               static_cast<int>(m_writers[i]->getProxiesCount()));
      }
      if (m_writers[i]->m_attributes.endpointGuid.entityId ==
          ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER) {
        printf(
            "Writer %u: SEDP PUBLICATION WRITER | Remote Proxies = %u   \r\n ",
            i, static_cast<int>(m_writers[i]->getProxiesCount()));
      }
      if (m_writers[i]->m_attributes.endpointGuid.entityId ==
          ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER) {
        printf(
            "Writer %u: SEDP SUBSCRIPTION WRITER | Remote Proxies = %u   \r\n ",
            i, static_cast<int>(m_writers[i]->getProxiesCount()));
      }
#endif
      continue;
    }

    if (m_writers[i] != nullptr && m_writers[i]->isInitialized()) {
      max_writer_proxies =
          std::max(max_writer_proxies, m_writers[i]->getProxiesCount());
#ifdef PARTICIPANT_PRINTINFO_LONG
      printf("Writer %u: Topic = %s | Type = %s | Remote Proxies = %u | SEDP "
             "SN = %u   \r\n ",
             i, m_writers[i]->m_attributes.topicName,
             m_writers[i]->m_attributes.typeName,
             static_cast<int>(m_writers[i]->getProxiesCount()),
             static_cast<int>(m_writers[i]->getSEDPSequenceNumber().low));
#endif
    }
  }

  printf("Max Writer Proxies %u \r\n ", max_writer_proxies);
  printf("Max Reader Proxies %u \r\n ", max_reader_proxies);
  printf("Unmatched Remote Readers = %u\r\n",
         static_cast<int>(m_sedpAgent.getNumRemoteUnmatchedReaders()));
  printf("Unmatched Remote Writers = %u \r\n ",
         static_cast<int>(m_sedpAgent.getNumRemoteUnmatchedWriters()));
  printf("Remote Participants = %u \r\n ",
         static_cast<int>(m_remoteParticipants.getNumElements()));
}

rtps::SPDPAgent &Participant::getSPDPAgent() { return m_spdpAgent; }

void Participant::addBuiltInEndpoints(BuiltInEndpoints &endpoints) {
  Lock lock{m_mutex};
  m_hasBuilInEndpoints = true;
  m_spdpAgent.init(*this, endpoints);
  m_sedpAgent.init(*this, endpoints);

  // This needs to be done after initializing the agents
  addWriter(endpoints.spdpWriter);
  addReader(endpoints.spdpReader);
  addWriter(endpoints.sedpPubWriter);
  addReader(endpoints.sedpPubReader);
  addWriter(endpoints.sedpSubWriter);
  addReader(endpoints.sedpSubReader);
}

void Participant::newMessage(const uint8_t *data, DataSize_t size) {
  if (!m_receiver.processMessage(data, size)) {
    PARTICIPANT_LOG("MESSAGE PROCESSING FAILE \r\n");
  }
}

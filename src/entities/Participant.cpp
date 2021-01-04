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

using rtps::Participant;

Participant::Participant()
    : m_guidPrefix(GUIDPREFIX_UNKNOWN), m_participantId(PARTICIPANT_ID_INVALID),
      m_receiver(this) {}
Participant::Participant(const GuidPrefix_t &guidPrefix,
                         ParticipantId_t participantId)
    : m_guidPrefix(guidPrefix), m_participantId(participantId),
      m_receiver(this) {}

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
  if (pWriter != nullptr && m_numWriters != m_writers.size()) {
    m_writers[m_numWriters++] = pWriter;
    if (m_hasBuilInEndpoints) {
      m_sedpAgent.addWriter(*pWriter);
    }
    return pWriter;
  } else {
    return nullptr;
  }
}

bool Participant::isWritersFull() { return m_numWriters == m_writers.size(); }

rtps::Reader *Participant::addReader(Reader *pReader) {
  if (pReader != nullptr && m_numReaders != m_readers.size()) {
    m_readers[m_numReaders++] = pReader;
    if (m_hasBuilInEndpoints) {
      m_sedpAgent.addReader(*pReader);
    }
    return pReader;
  } else {
    return nullptr;
  }
}

bool Participant::isReadersFull() { return m_numReaders == m_readers.size(); }

rtps::Writer *Participant::getWriter(EntityId_t id) const {
  for (uint8_t i = 0; i < m_numWriters; ++i) {
    if (m_writers[i]->m_attributes.endpointGuid.entityId == id) {
      return m_writers[i];
    }
  }
  return nullptr;
}

rtps::Reader *Participant::getReader(EntityId_t id) const {
  for (uint8_t i = 0; i < m_numReaders; ++i) {
    if (m_readers[i]->m_attributes.endpointGuid.entityId == id) {
      return m_readers[i];
    }
  }
  return nullptr;
}

rtps::Writer *
Participant::getMatchingWriter(const TopicData &readerTopicData) const {
  for (uint8_t i = 0; i < m_numWriters; ++i) {
    if (m_writers[i]->m_attributes.matchesTopicOf(readerTopicData) &&
        (readerTopicData.reliabilityKind == ReliabilityKind_t::BEST_EFFORT ||
         m_writers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::RELIABLE)) {
      return m_writers[i];
    }
  }
  return nullptr;
}

rtps::Reader *
Participant::getMatchingReader(const TopicData &writerTopicData) const {
  for (uint8_t i = 0; i < m_numReaders; ++i) {
    if (m_readers[i]->m_attributes.matchesTopicOf(writerTopicData) &&
        (writerTopicData.reliabilityKind == ReliabilityKind_t::RELIABLE ||
         m_readers[i]->m_attributes.reliabilityKind ==
             ReliabilityKind_t::BEST_EFFORT)) {
      return m_readers[i];
    }
  }
  return nullptr;
}

bool Participant::addNewRemoteParticipant(
    const ParticipantProxyData &remotePart) {
  return m_remoteParticipants.add(remotePart);
}

bool Participant::removeRemoteParticipant(const GuidPrefix_t &prefix) {
  auto isElementToRemove = [&](const ParticipantProxyData &proxy) {
    return proxy.m_guid.prefix == prefix;
  };
  auto thunk = [](void *arg, const ParticipantProxyData &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  return m_remoteParticipants.remove(thunk, &isElementToRemove);
}

const rtps::ParticipantProxyData *
Participant::findRemoteParticipant(const GuidPrefix_t &prefix) {
  auto isElementToFind = [&](const ParticipantProxyData &proxy) {
    return proxy.m_guid.prefix == prefix;
  };
  auto thunk = [](void *arg, const ParticipantProxyData &value) {
    return (*static_cast<decltype(isElementToFind) *>(arg))(value);
  };

  return m_remoteParticipants.find(thunk, &isElementToFind);
}

bool Participant::hasReaderWithMulticastLocator(ip4_addr_t address) {
  for (uint8_t i = 0; i < m_numReaders; i++) {
    if(m_readers[i]->m_attributes.multicastLocator.isSameAddress(&address)){
      return true;
    }
  }
  return false;
}

uint32_t Participant::getRemoteParticipantCount() {
  return m_remoteParticipants.getNumElements();
}

rtps::MessageReceiver *Participant::getMessageReceiver() { return &m_receiver; }

void Participant::addBuiltInEndpoints(BuiltInEndpoints &endpoints) {
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

  m_spdpAgent.start();
}

void Participant::newMessage(const uint8_t *data, DataSize_t size) {
  m_receiver.processMessage(data, size);
}

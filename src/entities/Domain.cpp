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

#include "rtps/entities/Domain.h"
#include "rtps/utils/Log.h"
#include "rtps/utils/udpUtils.h"

#define DOMAIN_VERBOSE 0

using rtps::Domain;

Domain::Domain()
    : m_threadPool(receiveJumppad, this),
      m_transport(ThreadPool::readCallback, &m_threadPool) {
  m_transport.createUdpConnection(getUserMulticastPort());
  m_transport.createUdpConnection(getBuiltInMulticastPort());
  m_transport.joinMultiCastGroup(transformIP4ToU32(239, 255, 0, 1));
}

bool Domain::completeInit() {
  m_initComplete = m_threadPool.startThreads();
#if DOMAIN_VERBOSE
  if (!m_initComplete) {
    printf("Domain: Failed starting threads\n");
  }
#endif
  return m_initComplete;
}

void Domain::stop() { m_threadPool.stopThreads(); }

void Domain::receiveJumppad(void *callee, const PacketInfo &packet) {
  auto domain = static_cast<Domain *>(callee);
  domain->receiveCallback(packet);
}

void Domain::receiveCallback(const PacketInfo &packet) {
  if (packet.buffer.firstElement->next != nullptr) {
#if DOMAIN_VERBOSE
    Log::printLine("Domain: Cannot handle multiple elements chained. You might "
                   "want to increase PBUF_POOL_BUFSIZE\n");
#endif
  }

  if (isMultiCastPort(packet.destPort)) {
    // Pass to all
#if DOMAIN_VERBOSE
    printf("Domain: Multicast to port %u\n", packet.destPort);
#endif
    for (auto i = 0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
      m_participants[i].newMessage(
          static_cast<uint8_t *>(packet.buffer.firstElement->payload),
          packet.buffer.firstElement->len);
    }
  } else {
    // Pass to addressed one only
    ParticipantId_t id = getParticipantIdFromUnicastPort(
        packet.destPort, isUserPort(packet.destPort));
    if (id != PARTICIPANT_ID_INVALID) {
#if DOMAIN_VERBOSE
      printf("Domain: Got unicast message on port %u\n", packet.destPort);
#endif
      if (id < m_nextParticipantId) {
        m_participants[id - PARTICIPANT_START_ID].newMessage(
            static_cast<uint8_t *>(packet.buffer.firstElement->payload),
            packet.buffer.firstElement->len);
      } else {
#if DOMAIN_VERBOSE
        printf("Domain: Participant id too high.\n");
#endif
      }
    } else {
#if DOMAIN_VERBOSE
      printf("Domain: Got message to port %u: no matching participant\n",
             packet.destPort);
#endif
    }
  }
}

rtps::Participant *Domain::createParticipant() {
#if DOMAIN_VERBOSE
  printf("Domain: Creating new participant.\n");
#endif
  auto nextSlot =
      static_cast<uint8_t>(m_nextParticipantId - PARTICIPANT_START_ID);
  if (m_initComplete || m_participants.size() <= nextSlot) {
    return nullptr;
  }

  auto &entry = m_participants[nextSlot];
  entry.reuse(generateGuidPrefix(m_nextParticipantId), m_nextParticipantId);
  registerPort(entry);
  createBuiltinWritersAndReaders(entry);
  ++m_nextParticipantId;
  return &entry;
}

void Domain::createBuiltinWritersAndReaders(Participant &part) {
  // SPDP
  StatelessWriter &spdpWriter = m_statelessWriters[m_numStatelessWriters++];
  StatelessReader &spdpReader = m_statelessReaders[m_numStatelessReaders++];

  TopicData spdpWriterAttributes;
  spdpWriterAttributes.topicName[0] = '\0';
  spdpWriterAttributes.typeName[0] = '\0';
  spdpWriterAttributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;
  spdpWriterAttributes.endpointGuid.prefix = part.m_guidPrefix;
  spdpWriterAttributes.endpointGuid.entityId =
      ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
  spdpWriterAttributes.unicastLocator = getBuiltInMulticastLocator();

  spdpWriter.init(spdpWriterAttributes, TopicKind_t::WITH_KEY, &m_threadPool,
                  m_transport);
  spdpWriter.addNewMatchedReader(
      ReaderProxy{{part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER},
                  getBuiltInMulticastLocator()});
  spdpReader.m_attributes.endpointGuid = {
      part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER};

  // SEDP
  StatefulReader &sedpPubReader = m_statefulReaders[m_numStatefulReaders++];
  StatefulReader &sedpSubReader = m_statefulReaders[m_numStatefulReaders++];
  StatefulWriter &sedpPubWriter = m_statefulWriters[m_numStatefulWriters++];
  StatefulWriter &sedpSubWriter = m_statefulWriters[m_numStatefulWriters++];

  // Prepare attributes
  TopicData sedpAttributes;
  sedpAttributes.topicName[0] = '\0';
  sedpAttributes.typeName[0] = '\0';
  sedpAttributes.reliabilityKind = ReliabilityKind_t::RELIABLE;
  sedpAttributes.endpointGuid.prefix = part.m_guidPrefix;
  sedpAttributes.unicastLocator =
      getBuiltInUnicastLocator(part.m_participantId);

  // READER
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
  sedpPubReader.init(sedpAttributes, m_transport);
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
  sedpSubReader.init(sedpAttributes, m_transport);

  // WRITER
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
  sedpPubWriter.init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool,
                     m_transport);

  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
  sedpSubWriter.init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool,
                     m_transport);

  // COLLECT
  BuiltInEndpoints endpoints{};
  endpoints.spdpWriter = &spdpWriter;
  endpoints.spdpReader = &spdpReader;
  endpoints.sedpPubReader = &sedpPubReader;
  endpoints.sedpSubReader = &sedpSubReader;
  endpoints.sedpPubWriter = &sedpPubWriter;
  endpoints.sedpSubWriter = &sedpSubWriter;

  part.addBuiltInEndpoints(endpoints);
}

void Domain::registerPort(const Participant &part) {
  m_transport.createUdpConnection(getUserUnicastPort(part.m_participantId));
  m_transport.createUdpConnection(getBuiltInUnicastPort(part.m_participantId));
}

rtps::Reader *Domain::readerExists(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
  if (reliable) {
    for (unsigned int i = 0; i < m_numStatefulReaders; i++) {
      if (m_statefulReaders[i].isInitialized()) {
        if (strncmp(m_statefulReaders[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statefulReaders[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

#if DOMAIN_VERBOSE
        printf("StatefulReader exists already [%s, %s]\n", topicName, typeName);
#endif

        return &m_statefulReaders[i];
      }
    }
  } else {
    for (unsigned int i = 0; i < m_numStatelessReaders; i++) {
      if (m_statelessReaders[i].isInitialized()) {
        if (strncmp(m_statelessReaders[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statelessReaders[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

#if DOMAIN_VERBOSE
        printf("StatelessReader exists [%s, %s]\n", topicName, typeName);
#endif
        return &m_statelessReaders[i];
      }
    }
  }

  return nullptr;
}

rtps::Writer *Domain::writerExists(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
  if (reliable) {
    for (unsigned int i = 0; i < m_numStatefulWriters; i++) {
      if (m_statefulWriters[i].isInitialized()) {
        if (strncmp(m_statefulWriters[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statefulWriters[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

#if DOMAIN_VERBOSE
        printf("StatefulWriter exists [%s, %s]\n", topicName, typeName);
#endif

        return &m_statefulWriters[i];
      }
    }
  } else {
    for (unsigned int i = 0; i < m_numStatelessWriters; i++) {
      if (m_statelessWriters[i].isInitialized()) {
        if (strncmp(m_statelessWriters[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statelessWriters[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

#if DOMAIN_VERBOSE
        printf("StatelessWriter exists [%s, %s]\n", topicName, typeName);
#endif
        return &m_statelessWriters[i];
      }
    }
  }

  return nullptr;
}

rtps::Writer *Domain::createWriter(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
#if DOMAIN_VERBOSE
  printf("Creating writer[%s, %s]\n", topicName, typeName);
#endif

  // Check if there is enough capacity for more writers
  if ((reliable && m_statefulWriters.size() <= m_numStatefulWriters) ||
      (!reliable && m_statelessWriters.size() <= m_numStatelessWriters) ||
      part.isWritersFull()) {
    return nullptr;
  }

  // TODO Distinguish WithKey and NoKey (Also changes EntityKind)
  TopicData attributes;

  if (strlen(topicName) > Config::MAX_TOPICNAME_LENGTH ||
      strlen(typeName) > Config::MAX_TYPENAME_LENGTH) {
    return nullptr;
  }
  strcpy(attributes.topicName, topicName);
  strcpy(attributes.typeName, typeName);
  attributes.endpointGuid.prefix = part.m_guidPrefix;
  attributes.endpointGuid.entityId = {
      part.getNextUserEntityKey(),
      EntityKind_t::USER_DEFINED_WRITER_WITHOUT_KEY};
  attributes.unicastLocator = getUserUnicastLocator(part.m_participantId);

#if DOMAIN_VERBOSE
  printf("Creating writer[%s, %s]\n", topicName, typeName);
#endif
  if (reliable) {
    attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

    StatefulWriter &writer = m_statefulWriters[m_numStatefulWriters++];
    writer.init(attributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

    part.addWriter(&writer);
    return &writer;
  } else {
    attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

    StatelessWriter &writer = m_statelessWriters[m_numStatelessWriters++];
    writer.init(attributes, TopicKind_t::NO_KEY, &m_threadPool, m_transport);

    part.addWriter(&writer);
    return &writer;
  }
}

rtps::Reader *Domain::createReader(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
#if DOMAIN_VERBOSE
  printf("Creating reader[%s, %s]\n", topicName, typeName);
#endif
  if ((reliable && m_statefulReaders.size() <= m_numStatefulReaders) ||
      (!reliable && m_statelessReaders.size() <= m_numStatelessReaders) ||
      part.isReadersFull()) {
    return nullptr;
  }

  // TODO Distinguish WithKey and NoKey (Also changes EntityKind)
  TopicData attributes;

  if (strlen(topicName) > Config::MAX_TOPICNAME_LENGTH ||
      strlen(typeName) > Config::MAX_TYPENAME_LENGTH) {
    return nullptr;
  }
  strcpy(attributes.topicName, topicName);
  strcpy(attributes.typeName, typeName);
  attributes.endpointGuid.prefix = part.m_guidPrefix;
  attributes.endpointGuid.entityId = {
      part.getNextUserEntityKey(),
      EntityKind_t::USER_DEFINED_READER_WITHOUT_KEY};
  attributes.unicastLocator = getUserUnicastLocator(part.m_participantId);

#if DOMAIN_VERBOSE
  printf("Creating reader[%s, %s]\n", topicName, typeName);
#endif

  if (reliable) {
    if (m_numStatefulReaders == m_statefulReaders.size()) {
      return nullptr;
    }

    attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

    StatefulReader &reader = m_statefulReaders[m_numStatefulReaders++];
    reader.init(attributes, m_transport);

    if (!part.addReader(&reader)) {
      return nullptr;
    }
    return &reader;
  } else {
    if (m_numStatelessReaders == m_statelessReaders.size()) {
      return nullptr;
    }

    attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

    StatelessReader &reader = m_statelessReaders[m_numStatelessReaders++];
    reader.init(attributes);

    if (!part.addReader(&reader)) {
      return nullptr;
    }
    return &reader;
  }
}

rtps::GuidPrefix_t Domain::generateGuidPrefix(ParticipantId_t id) const {
  GuidPrefix_t prefix = Config::BASE_GUID_PREFIX;
  prefix.id[prefix.id.size() - 1] = *reinterpret_cast<uint8_t *>(&id);
  return prefix;
}

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

#if DOMAIN_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef DOMAIN_LOG
#define DOMAIN_LOG(...)                                                        \
  if (true) {                                                                  \
    printf("[Domain] ");                                                       \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#endif
#else
#define DOMAIN_LOG(...) //
#endif

using rtps::Domain;

Domain::Domain()
    : m_threadPool(receiveJumppad, this),
      m_transport(ThreadPool::readCallback, &m_threadPool) {
  m_transport.createUdpConnection(getUserMulticastPort());
  m_transport.createUdpConnection(getBuiltInMulticastPort());
  m_transport.joinMultiCastGroup(transformIP4ToU32(239, 255, 0, 1));
  createMutex(&m_mutex);
}

Domain::~Domain() { stop(); }

bool Domain::completeInit() {
  m_initComplete = m_threadPool.startThreads();

  if (!m_initComplete) {
    DOMAIN_LOG("Failed starting threads\n");
  }

  for (auto i = 0; i < m_nextParticipantId; i++) {
    m_participants[i].getSPDPAgent().start();
  }
  return m_initComplete;
}

void Domain::stop() { m_threadPool.stopThreads(); }

void Domain::receiveJumppad(void *callee, const PacketInfo &packet) {
  auto domain = static_cast<Domain *>(callee);
  domain->receiveCallback(packet);
}

void Domain::receiveCallback(const PacketInfo &packet) {
  if (packet.buffer.firstElement->next != nullptr) {

    DOMAIN_LOG("Cannot handle multiple elements chained. You might "
               "want to increase PBUF_POOL_BUFSIZE\n");
  }

  if (isMetaMultiCastPort(packet.destPort)) {
    // Pass to all
    DOMAIN_LOG("Domain: Multicast to port %u\n", packet.destPort);
    for (auto i = 0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
      m_participants[i].newMessage(
          static_cast<uint8_t *>(packet.buffer.firstElement->payload),
          packet.buffer.firstElement->len);
    }
    // First Check if UserTraffic Multicast
  } else if (isUserMultiCastPort(packet.destPort)) {
    // Pass to Participant with assigned Multicast Adress (Port ist everytime
    // the same)
    DOMAIN_LOG("Domain: Got user multicast message on port %u\n",
               packet.destPort);
    for (auto i = 0; i < m_nextParticipantId - PARTICIPANT_START_ID; ++i) {
      if (m_participants[i].hasReaderWithMulticastLocator(packet.destAddr)) {
        DOMAIN_LOG("Domain: Forward Multicast only to Participant: %u\n", i);
        m_participants[i].newMessage(
            static_cast<uint8_t *>(packet.buffer.firstElement->payload),
            packet.buffer.firstElement->len);
      }
    }
  } else {
    // Pass to addressed one only (Unicast, by Port)
    ParticipantId_t id = getParticipantIdFromUnicastPort(
        packet.destPort, isUserPort(packet.destPort));
    if (id != PARTICIPANT_ID_INVALID) {
      DOMAIN_LOG("Domain: Got unicast message on port %u\n", packet.destPort);
      if (id < m_nextParticipantId &&
          id >= PARTICIPANT_START_ID) { // added extra check to avoid segfault
                                        // (id below START_ID)
        m_participants[id - PARTICIPANT_START_ID].newMessage(
            static_cast<uint8_t *>(packet.buffer.firstElement->payload),
            packet.buffer.firstElement->len);
      } else {
        DOMAIN_LOG("Domain: Participant id too high or unplausible.\n");
      }
    } else {
      DOMAIN_LOG("Domain: Got message to port %u: no matching participant\n",
                 packet.destPort);
    }
  }
}

rtps::Participant *Domain::createParticipant() {

  DOMAIN_LOG("Domain: Creating new participant.\n");

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
  StatelessWriter *spdpWriter =
      getNextUnusedEndpoint<decltype(m_statelessWriters), StatelessWriter>(
          m_statelessWriters);
  StatelessReader *spdpReader =
      getNextUnusedEndpoint<decltype(m_statelessReaders), StatelessReader>(
          m_statelessReaders);

  TopicData spdpWriterAttributes;
  spdpWriterAttributes.topicName[0] = '\0';
  spdpWriterAttributes.typeName[0] = '\0';
  spdpWriterAttributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;
  spdpWriterAttributes.durabilityKind = DurabilityKind_t::TRANSIENT_LOCAL;
  spdpWriterAttributes.endpointGuid.prefix = part.m_guidPrefix;
  spdpWriterAttributes.endpointGuid.entityId =
      ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
  spdpWriterAttributes.unicastLocator = getBuiltInMulticastLocator();

  spdpWriter->init(spdpWriterAttributes, TopicKind_t::WITH_KEY, &m_threadPool,
                   m_transport);
  spdpWriter->addNewMatchedReader(
      ReaderProxy{{part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER},
                  getBuiltInMulticastLocator(),
                  false});

  TopicData spdpReaderAttributes;
  spdpReaderAttributes.endpointGuid = {
      part.m_guidPrefix, ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER};
  spdpReader->init(spdpReaderAttributes);

  // SEDP

  // Prepare attributes
  TopicData sedpAttributes;
  sedpAttributes.topicName[0] = '\0';
  sedpAttributes.typeName[0] = '\0';
  sedpAttributes.reliabilityKind = ReliabilityKind_t::RELIABLE;
  sedpAttributes.durabilityKind = DurabilityKind_t::TRANSIENT_LOCAL;
  sedpAttributes.endpointGuid.prefix = part.m_guidPrefix;
  sedpAttributes.unicastLocator =
      getBuiltInUnicastLocator(part.m_participantId);

  // READER
  StatefulReader *sedpPubReader =
      getNextUnusedEndpoint<decltype(m_statefulReaders), StatefulReader>(
          m_statefulReaders);
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
  sedpPubReader->init(sedpAttributes, m_transport);

  StatefulReader *sedpSubReader =
      getNextUnusedEndpoint<decltype(m_statefulReaders), StatefulReader>(
          m_statefulReaders);
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
  sedpSubReader->init(sedpAttributes, m_transport);

  // WRITER
  StatefulWriter *sedpPubWriter =
      getNextUnusedEndpoint<decltype(m_statefulWriters), StatefulWriter>(
          m_statefulWriters);
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
  sedpPubWriter->init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool,
                      m_transport);

  StatefulWriter *sedpSubWriter =
      getNextUnusedEndpoint<decltype(m_statefulWriters), StatefulWriter>(
          m_statefulWriters);
  sedpAttributes.endpointGuid.entityId =
      ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
  sedpSubWriter->init(sedpAttributes, TopicKind_t::NO_KEY, &m_threadPool,
                      m_transport);

  // COLLECT
  BuiltInEndpoints endpoints{};
  endpoints.spdpWriter = spdpWriter;
  endpoints.spdpReader = spdpReader;
  endpoints.sedpPubReader = sedpPubReader;
  endpoints.sedpSubReader = sedpSubReader;
  endpoints.sedpPubWriter = sedpPubWriter;
  endpoints.sedpSubWriter = sedpSubWriter;

  part.addBuiltInEndpoints(endpoints);
}

void Domain::registerPort(const Participant &part) {
  m_transport.createUdpConnection(getUserUnicastPort(part.m_participantId));
  m_transport.createUdpConnection(getBuiltInUnicastPort(part.m_participantId));
  m_threadPool.addBuiltinPort(getBuiltInUnicastPort(part.m_participantId));
}

void Domain::registerMulticastPort(FullLengthLocator mcastLocator) {
  if (mcastLocator.kind == LocatorKind_t::LOCATOR_KIND_UDPv4) {
    m_transport.createUdpConnection(mcastLocator.getLocatorPort());
  }
}

rtps::Reader *Domain::readerExists(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
  Lock lock{m_mutex};
  if (reliable) {
    for (unsigned int i = 0; i < m_statefulReaders.size(); i++) {
      if (m_statefulReaders[i].isInitialized()) {
        if (strncmp(m_statefulReaders[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statefulReaders[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        DOMAIN_LOG("StatefulReader exists already [%s, %s]\n", topicName,
                   typeName);

        return &m_statefulReaders[i];
      }
    }
  } else {
    for (unsigned int i = 0; i < m_statelessReaders.size(); i++) {
      if (m_statelessReaders[i].isInitialized()) {
        if (strncmp(m_statelessReaders[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statelessReaders[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        DOMAIN_LOG("StatelessReader exists [%s, %s]\n", topicName, typeName);

        return &m_statelessReaders[i];
      }
    }
  }

  return nullptr;
}

rtps::Writer *Domain::writerExists(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable) {
  Lock lock{m_mutex};
  if (reliable) {
    for (unsigned int i = 0; i < m_statefulWriters.size(); i++) {
      if (m_statefulWriters[i].isInitialized()) {
        if (strncmp(m_statefulWriters[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statefulWriters[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        DOMAIN_LOG("StatefulWriter exists [%s, %s]\n", topicName, typeName);

        return &m_statefulWriters[i];
      }
    }
  } else {
    for (unsigned int i = 0; i < m_statelessWriters.size(); i++) {
      if (m_statelessWriters[i].isInitialized()) {
        if (strncmp(m_statelessWriters[i].m_attributes.topicName, topicName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        if (strncmp(m_statelessWriters[i].m_attributes.typeName, typeName,
                    Config::MAX_TYPENAME_LENGTH) != 0) {
          continue;
        }

        DOMAIN_LOG("StatelessWriter exists [%s, %s]\n", topicName, typeName);

        return &m_statelessWriters[i];
      }
    }
  }

  return nullptr;
}

rtps::Writer *Domain::createWriter(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable,
                                   bool enforceUnicast) {
  Lock lock{m_mutex};
  StatelessWriter *statelessWriter =
      getNextUnusedEndpoint<decltype(m_statelessWriters), StatelessWriter>(
          m_statelessWriters);
  StatefulWriter *statefulWriter =
      getNextUnusedEndpoint<decltype(m_statefulWriters), StatefulWriter>(
          m_statefulWriters);

  // Check if there is enough capacity for more writers
  if ((reliable && statefulWriter == nullptr) ||
      (!reliable && statelessWriter == nullptr) || part.isWritersFull()) {

    DOMAIN_LOG("No Writer created. Max Number of Writers reached.\n");

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
  attributes.durabilityKind = DurabilityKind_t::TRANSIENT_LOCAL;

  DOMAIN_LOG("Creating writer[%s, %s]\n", topicName, typeName);

  if (reliable) {
    attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

    statefulWriter->init(attributes, TopicKind_t::NO_KEY, &m_threadPool,
                         m_transport, enforceUnicast);

    if (!part.addWriter(statefulWriter)) {
      return nullptr;
    }
    return statefulWriter;
  } else {
    attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

    statelessWriter->init(attributes, TopicKind_t::NO_KEY, &m_threadPool,
                          m_transport, enforceUnicast);

    if (!part.addWriter(statelessWriter)) {
      return nullptr;
    }
    return statelessWriter;
  }
}

rtps::Reader *Domain::createReader(Participant &part, const char *topicName,
                                   const char *typeName, bool reliable,
                                   ip4_addr_t mcastaddress) {
  Lock lock{m_mutex};
  StatelessReader *statelessReader =
      getNextUnusedEndpoint<decltype(m_statelessReaders), StatelessReader>(
          m_statelessReaders);
  StatefulReader *statefulReader =
      getNextUnusedEndpoint<decltype(m_statefulReaders), StatefulReader>(
          m_statefulReaders);

  if ((reliable && statefulReader == nullptr) ||
      (!reliable && statelessReader == nullptr) || part.isReadersFull()) {

    DOMAIN_LOG("No Reader created. Max Number of Readers reached.\n");

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
  if (!isZeroAddress(mcastaddress)) {
    if (ip4_addr_ismulticast(&mcastaddress)) {
      attributes.multicastLocator = rtps::FullLengthLocator::createUDPv4Locator(
          ip4_addr1(&mcastaddress), ip4_addr2(&mcastaddress),
          ip4_addr3(&mcastaddress), ip4_addr4(&mcastaddress),
          getUserMulticastPort());
      m_transport.joinMultiCastGroup(
          attributes.multicastLocator.getIp4Address());
      registerMulticastPort(attributes.multicastLocator);

      DOMAIN_LOG("Multicast enabled!\n");

    } else {

      DOMAIN_LOG("This is not a Multicastaddress!\n");
    }
  }
  attributes.durabilityKind = DurabilityKind_t::VOLATILE;

  DOMAIN_LOG("Creating reader[%s, %s]\n", topicName, typeName);

  if (reliable) {

    attributes.reliabilityKind = ReliabilityKind_t::RELIABLE;

    statefulReader->init(attributes, m_transport);

    if (!part.addReader(statefulReader)) {
      DOMAIN_LOG("Failed to add reader to participant.\n");

      return nullptr;
    }
    return statefulReader;
  } else {

    attributes.reliabilityKind = ReliabilityKind_t::BEST_EFFORT;

    statelessReader->init(attributes);

    if (!part.addReader(statelessReader)) {
      return nullptr;
    }
    return statelessReader;
  }
}

bool rtps::Domain::deleteReader(Participant &part, Reader *reader) {
  Lock lock{m_mutex};
  if(reader == nullptr || !reader->isInitialized()){
	  return false;
  }
  if (!part.deleteReader(reader)) {
    return false;
  }

  reader->reset();
  return true;
}

bool rtps::Domain::deleteWriter(Participant &part, Writer *writer) {
  Lock lock{m_mutex};
  if(writer == nullptr || !writer->isInitialized()){
	  return false;
  }
  if (!part.deleteWriter(writer)) {
    return false;
  }

  writer->reset();
  return true;
}

void rtps::Domain::printInfo() {
  for (unsigned int i = 0; i < m_participants.size(); i++) {
    DOMAIN_LOG("Participant %u\r\n", i);
    m_participants[i].printInfo();
  }
}

rtps::GuidPrefix_t Domain::generateGuidPrefix(ParticipantId_t id) const {
  GuidPrefix_t prefix;
  if (Config::BASE_GUID_PREFIX == GUID_RANDOM) {
    for (unsigned int i = 0; i < rtps::Config::BASE_GUID_PREFIX.id.size();
         i++) {
      prefix.id[i] = rand();
    }
  } else {
    for (unsigned int i = 0; i < rtps::Config::BASE_GUID_PREFIX.id.size();
         i++) {
      prefix.id[i] = Config::BASE_GUID_PREFIX.id[i];
    }
  }
  return prefix;
}

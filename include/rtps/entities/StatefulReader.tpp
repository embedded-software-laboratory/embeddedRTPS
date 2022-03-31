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

#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "rtps/entities/StatefulReader.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/utils/Lock.h"

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define SFR_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatefulReader %s] ", &m_attributes.topicName[0]);                \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define SFR_LOG(...) //
#endif

using rtps::StatefulReaderT;

template <class NetworkDriver>
StatefulReaderT<NetworkDriver>::~StatefulReaderT() {
  //  if(sys_mutex_valid(&m_mutex)){ // Getting invalid pointer error, there
  //  seems sth strange
  //    sys_mutex_free(&m_mutex);
  //  }
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::init(const TopicData &attributes,
                                          NetworkDriver &driver) {
  if (sys_mutex_new(&m_mutex) != ERR_OK) {

    SFR_LOG("StatefulReader: Failed to create mutex.\n");

    return;
  }

  if(attributes.endpointGuid.entityId.entityKind == EntityKind_t::USER_DEFINED_READER_WITH_KEY){
    m_kind = TopicKind_t::WITH_KEY;
  }

  m_attributes = attributes;
  m_transport = &driver;
  m_packetInfo.srcPort = attributes.unicastLocator.port;
  m_is_initialized_ = true;
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::isOwner(InstanceHandle_t &handle, WriterProxy *proxy){
  for(auto &instance : m_instances){
    if(instance.handle == handle){
      if(instance.owner == nullptr){
        instance.owner = proxy;
        return true;
      }
      if(instance.owner == proxy){
        return true;
      }
      else{
        if(proxy->ownershipStrength < instance.owner->ownershipStrength){
          return false;
        }
        else if(proxy->ownershipStrength > instance.owner->ownershipStrength){
          instance.owner = proxy;
          return true;
        }
        else{//equal strength , just pick the first one
          return false;
        }
      }
    }
  }
  Instance_t instance;
  instance.owner = proxy;
  instance.handle = handle;
  m_instances.add(instance);
  return true;
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::newChange(
    const ReaderCacheChange &cacheChange) {
  if (m_callback == nullptr) {
    return;
  }
  Lock lock{m_mutex};
  for (auto &proxy : m_proxies) {
    if (proxy.remoteWriterGuid == cacheChange.writerGuid) {
      if (proxy.expectedSN == cacheChange.sn) {
        ++proxy.expectedSN;
        if(m_attributes.ownership_Kind == OwnershipKind_t::EXCLUSIVE) {
          InstanceHandle_t handle;
          m_KeyCallback(cacheChange.getData(), cacheChange.getDataSize(), handle);
          if (isOwner(handle, &proxy)) { //
            m_callback(m_callee, cacheChange);
          }
        }
        else{
          m_callback(m_callee, cacheChange);
        }
        return;
      }
    }
  }
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::registerCallback(ddsReaderCallback_fp cb,
                                                      void *callee) {
  if (cb != nullptr) {
    m_callback = cb;
    m_callee = callee; // It's okay if this is null
  } else {

    SFR_LOG("Passed callback is nullptr\n");
  }
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::registerKeyCallback(ddsGetKey_Callback_fp cb){
  if(cb != nullptr){
    m_KeyCallback = cb;
  }
  else{
    SFR_LOG("Passed Key callback is nullptr\n");
  }
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::addNewMatchedWriter(
    const WriterProxy &newProxy) {
#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
  SFR_LOG("New writer added with id: ");
  printGuid(newProxy.remoteWriterGuid);
  SFR_LOG("\n");
#endif
  return m_proxies.add(newProxy);
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::removeWriter(const Guid_t &guid) {
  Lock lock(m_mutex);
  auto isElementToRemove_Instance = [&](const Instance_t &instance){
      return instance.owner->remoteWriterGuid == guid;
  };
  auto thunk_instance = [](void *arg, const Instance_t &value) {
      return (*static_cast<decltype(isElementToRemove_Instance) *>(arg))(value);
  };
  m_instances.remove(thunk_instance, &isElementToRemove_Instance);

  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid == guid;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::removeWriterOfParticipant(
    const GuidPrefix_t &guidPrefix) {
  Lock lock(m_mutex);
  auto isElementToRemove_Instance = [&](const Instance_t &instance){
      return instance.owner->remoteWriterGuid.prefix == guidPrefix;
  };
  auto thunk_instance = [](void *arg, const Instance_t &value) {
      return (*static_cast<decltype(isElementToRemove_Instance) *>(arg))(value);
  };
  m_instances.remove(thunk_instance, &isElementToRemove_Instance);

  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid.prefix == guidPrefix;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::onNewHeartbeat(
    const SubmessageHeartbeat &msg, const GuidPrefix_t &sourceGuidPrefix) {
  Lock lock(m_mutex);
  PacketInfo info;
  info.srcPort = m_packetInfo.srcPort;
  WriterProxy *writer = nullptr;
  // Search for writer
  for (WriterProxy &proxy : m_proxies) {
    if (proxy.remoteWriterGuid.prefix == sourceGuidPrefix &&
        proxy.remoteWriterGuid.entityId == msg.writerId) {
      writer = &proxy;
      break;
    }
  }

  if (writer == nullptr) {

#if SFR_VERBOSE && RTPS_GLOBAL_VERBOSE
    SFR_LOG("Ignore heartbeat. Couldn't find a matching "
            "writer with id:");
    printEntityId(msg.writerId);
    SFR_LOG("\n");
#endif
    return false;
  }

  if (msg.count.value <= writer->hbCount.value) {

    SFR_LOG("Ignore heartbeat. Count too low.\n");
    return false;
  }

  writer->hbCount.value = msg.count.value;
  info.destAddr = writer->remoteLocator.getIp4Address();
  info.destPort = writer->remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
                                   writer->getMissing(msg.firstSN, msg.lastSN),
                                   writer->getNextAckNackCount(), false);

  SFR_LOG("Sending acknack.\n");
  m_transport->sendPacket(info);
  return true;
}

#undef SFR_VERBOSE

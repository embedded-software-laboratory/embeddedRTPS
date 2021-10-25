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

#include "rtps/entities/StatelessReader.h"
#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

using rtps::StatelessReader;

#if SLR_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define SLR_LOG(...)                                                           \
  if (true) {                                                                  \
    printf("[StatelessReader %s] ", &m_attributes.topicName[0]);               \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define SLR_LOG(...) //
#endif

void StatelessReader::init(const TopicData &attributes) {
  m_attributes = attributes;
  m_is_initialized_ = true;
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
    SLR_LOG("Failed to create mutex.\n");
  }
}

bool StatelessReader::isOwner(InstanceHandle_t &handle, WriterProxy *proxy){
  for(auto &instance : m_instances){
    bool keyEqual = instance.handle == handle;
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
  }//Instance not knwon
  Instance_t instance;
  instance.owner = proxy;
  instance.handle = handle;
  m_instances.add(instance);
  return true;
}

void StatelessReader::newChange(const ReaderCacheChange &cacheChange) {
  if (m_callback != nullptr) {
    if(m_attributes.ownership_Kind == OwnershipKind_t::EXCLUSIVE) {
      InstanceHandle_t handle;
      m_keyCallback(cacheChange.getData(), cacheChange.getDataSize(), handle);
      for (auto &proxy:m_proxies) {
        if (cacheChange.writerGuid == proxy.remoteWriterGuid && isOwner(handle, &proxy)) { //
          m_callback(m_callee, cacheChange);
        }
      }
    }
    else{
      m_callback(m_callee, cacheChange);
    }
  }
}

void StatelessReader::registerCallback(ddsReaderCallback_fp cb, void *callee) {
  if (cb != nullptr) {
    m_callback = cb;
    m_callee = callee; // It's okay if this is null
  } else {
#if SLR_VERBOSE
    SLR_LOG("Passed callback is nullptr\n");
#endif
  }
}

bool StatelessReader::addNewMatchedWriter(const WriterProxy &newProxy) {
#if (SLR_VERBOSE && RTPS_GLOBAL_VERBOSE)
  SLR_LOG("Adding WriterProxy");
  printGuid(newProxy.remoteWriterGuid);
  printf("\n");
#endif SLR_VERBOSE
  return m_proxies.add(newProxy);
}

void StatelessReader::removeWriter(const Guid_t &guid) {
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

void StatelessReader::removeWriterOfParticipant(
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

bool StatelessReader::onNewHeartbeat(const SubmessageHeartbeat &,
                                     const GuidPrefix_t &) {
  // nothing to do
  return true;
}

void StatelessReader::registerKeyCallback(ddsGetKey_Callback_fp cb){
  if(cb != nullptr){
    m_keyCallback = cb;
  }
}

#undef SLR_VERBOSE

#include <rtps/entities/Reader.h>
#include <rtps/entities/StatefulReader.h>
#include <rtps/entities/StatelessReader.h>
#include <rtps/utils/Lock.h>

using namespace rtps;

void Reader::executeCallbacks(const ReaderCacheChange &cacheChange) {
  Lock lock(m_mutex);
  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i] != nullptr) {
      m_callbacks[i](m_callback_arg[i], cacheChange);
    }
  }
}

void Reader::registerCallback(ddsReaderCallback_fp cb, void *arg) {
  Lock lock(m_mutex);
  if (m_callback_count == m_callbacks.size() || cb == nullptr) {
    return;
  }

  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i] == nullptr) {
      m_callbacks[i] = cb;
      m_callback_arg[i] = arg;
      m_callback_count++;
      return;
    }
  }
}

void Reader::removeCallback(ddsReaderCallback_fp cb) {
  Lock lock(m_mutex);
  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i] == cb) {
      m_callbacks[i] = nullptr;
      m_callback_arg[i] = nullptr;
      m_callback_count--;
      return;
    }
  }
}

void Reader::removeWriterOfParticipant(const GuidPrefix_t &guidPrefix) {
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid.prefix == guidPrefix;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

void Reader::removeWriter(const Guid_t &guid) {
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid == guid;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

bool Reader::addNewMatchedWriter(const WriterProxy &newProxy) {
  Lock lock(m_mutex);
#if (SFR_VERBOSE || SLR_VERBOSE) && RTPS_GLOBAL_VERBOSE
  SFR_LOG("New writer added with id: ");
  printGuid(newProxy.remoteWriterGuid);
  SFR_LOG("\n");
#endif
  return m_proxies.add(newProxy);
}

#include <rtps/entities/Reader.h>
#include <rtps/entities/StatefulReader.h>
#include <rtps/entities/StatelessReader.h>
#include <rtps/utils/Lock.h>
#include <rtps/utils/Log.h>

using namespace rtps;

Reader::Reader() { m_callbacks.fill({nullptr, nullptr, 0}); }

void Reader::executeCallbacks(const ReaderCacheChange &cacheChange) {
  Lock lock{m_callback_mutex};
  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i].function != nullptr) {
      m_callbacks[i].function(m_callbacks[i].arg, cacheChange);
    }
  }
}

bool Reader::initMutex() {
  if (m_proxies_mutex == nullptr) {
    if (!createMutex(&m_proxies_mutex)) {
      SFR_LOG("StatefulReader: Failed to create mutex.\n");
      return false;
    }
  }

  if (m_callback_mutex == nullptr) {
    if (!createMutex(&m_callback_mutex)) {
      SFR_LOG("StatefulReader: Failed to create mutex.\n");
      return false;
    }
  }

  return true;
}

void Reader::reset() {
  Lock lock1{m_proxies_mutex};
  Lock lock2{m_callback_mutex};

  m_proxies.clear();
  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    m_callbacks[i].function = nullptr;
    m_callbacks[i].arg = nullptr;
  }

  m_callback_count = 0;
  m_is_initialized_ = false;
}

bool Reader::isProxy(const Guid_t &guid) {
  for (const auto &proxy : m_proxies) {
    if (proxy.remoteWriterGuid.operator==(guid)) {
      return true;
    }
  }
  return false;
}

WriterProxy *Reader::getProxy(Guid_t guid) {
  auto isElementToFind = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid == guid;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToFind) *>(arg))(value);
  };
  return m_proxies.find(thunk, &isElementToFind);
}

Reader::callbackIdentifier_t
Reader::registerCallback(Reader::callbackFunction_t cb, void *arg) {
  Lock lock{m_callback_mutex};
  if (m_callback_count == m_callbacks.size() || cb == nullptr) {
    return false;
  }

  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i].function == nullptr) {
      m_callbacks[i].function = cb;
      m_callbacks[i].arg = arg;
      m_callbacks[i].identifier = m_callback_identifier++;
      m_callback_count++;
      return m_callbacks[i].identifier;
    }
  }

  return 0;
}

uint32_t Reader::getProxiesCount() { return m_proxies.getNumElements(); }

bool Reader::removeCallback(Reader::callbackIdentifier_t identifier) {
  Lock lock{m_callback_mutex};
  for (unsigned int i = 0; i < m_callbacks.size(); i++) {
    if (m_callbacks[i].identifier == identifier) {
      m_callbacks[i].function = nullptr;
      m_callbacks[i].arg = nullptr;
      m_callback_count--;
      return true;
    }
  }

  return false;
}

uint8_t Reader::getNumCallbacks() { return m_callback_count; }

void Reader::removeAllProxiesOfParticipant(const GuidPrefix_t &guidPrefix) {
  Lock lock{m_proxies_mutex};
  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid.prefix == guidPrefix;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

bool Reader::removeProxy(const Guid_t &guid) {
  Lock lock{m_proxies_mutex};
  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid == guid;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  return m_proxies.remove(thunk, &isElementToRemove);
}

bool Reader::addNewMatchedWriter(const WriterProxy &newProxy) {
  Lock lock{m_proxies_mutex};
#if (SFR_VERBOSE || SLR_VERBOSE) && RTPS_GLOBAL_VERBOSE
  char buffer[64];
  guid2Str(newProxy.remoteWriterGuid, buffer, sizeof(buffer));
  SFR_LOG("New writer added with id: %s", buffer);
#endif
  return m_proxies.add(newProxy);
}

void rtps::Reader::setSEDPSequenceNumber(const SequenceNumber_t &sn) {
  m_sedp_sequence_number = sn;
}
const rtps::SequenceNumber_t &rtps::Reader::getSEDPSequenceNumber() {
  return m_sedp_sequence_number;
}

int rtps::Reader::dumpAllProxies(dumpProxyCallback target, void *arg) {
  if (target == nullptr) {
    return 0;
  }
  Lock lock{m_proxies_mutex};
  int dump_count = 0;
  for (auto it = m_proxies.begin(); it != m_proxies.end(); ++it, ++dump_count) {
    target(this, *it, arg);
  }
  return dump_count;
}

bool rtps::Reader::sendPreemptiveAckNack(const WriterProxy &writer) {
  return true;
}

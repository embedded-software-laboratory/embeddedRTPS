#include <rtps/entities/Writer.h>

bool rtps::Writer::addNewMatchedReader(
    const ReaderProxy &newProxy)
{
  INIT_GUARD();
#if SFW_VERBOSE && RTPS_GLOBAL_VERBOSE
  SFW_LOG("New reader added with id: ");
  printGuid(newProxy.remoteReaderGuid);
#endif
  bool success = m_proxies.add(newProxy);
  if (!m_enforceUnicast)
  {
    manageSendOptions();
  }
  return success;
}

void rtps::Writer::removeReader(const Guid_t &guid)
{
  INIT_GUARD()
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const ReaderProxy &proxy)
  {
    return proxy.remoteReaderGuid == guid;
  };
  auto thunk = [](void *arg, const ReaderProxy &value)
  {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
  resetSendOptions();
}

void rtps::Writer::resetSendOptions()
{
  INIT_GUARD()
  for (auto &proxy : m_proxies)
  {
    proxy.suppressUnicast = false;
    proxy.useMulticast = false;
    proxy.unknown_eid = false;
  }
  manageSendOptions();
}

void rtps::Writer::manageSendOptions()
{
  INIT_GUARD();
  for (auto &proxy : m_proxies)
  {
    if (proxy.remoteMulticastLocator.kind ==
        LocatorKind_t::LOCATOR_KIND_INVALID)
    {
      proxy.suppressUnicast = false;
      proxy.useMulticast = false;
    }
    else
    {
      bool found = false;
      for (auto &avproxy : m_proxies)
      {
        if (avproxy.remoteMulticastLocator.kind ==
                LocatorKind_t::LOCATOR_KIND_UDPv4 &&
            avproxy.remoteMulticastLocator.getIp4Address().addr ==
                proxy.remoteMulticastLocator.getIp4Address().addr &&
            avproxy.remoteLocator.getIp4Address().addr !=
                proxy.remoteLocator.getIp4Address().addr)
        {
          if (avproxy.suppressUnicast == false)
          {
            avproxy.useMulticast = false;
            avproxy.suppressUnicast = true;
            proxy.useMulticast = true;
            proxy.suppressUnicast = true;
            if (avproxy.remoteReaderGuid.entityId !=
                proxy.remoteReaderGuid.entityId)
            {
              proxy.unknown_eid = true;
            }
          }
          found = true;
        }
      }
      if (!found)
      {
        proxy.useMulticast = false;
        proxy.suppressUnicast = false;
      }
    }
  }
}


void rtps::Writer::removeReaderOfParticipant(
    const GuidPrefix_t &guidPrefix) {
  INIT_GUARD();
  Lock lock(m_mutex);
  auto isElementToRemove = [&](const ReaderProxy &proxy) {
    return proxy.remoteReaderGuid.prefix == guidPrefix;
  };
  auto thunk = [](void *arg, const ReaderProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
  resetSendOptions();
}

bool rtps::Writer::isIrrelevant(ChangeKind_t kind) const {
  // Right now we only allow alive changes
  // return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY
  // && kind != ChangeKind_t::ALIVE);
  return kind != ChangeKind_t::ALIVE;
}

bool rtps::Writer::isInitialized() { return m_is_initialized_; }


void rtps::Writer::setSEDPSequenceNumber(const SequenceNumber_t& sn){
	m_sedp_sequence_number = sn;
}

const rtps::SequenceNumber_t* rtps::Writer::getSEDPSequenceNumber(){
	return &m_sedp_sequence_number;
}


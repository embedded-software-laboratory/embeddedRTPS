/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullReader.h"
#include "rtps/messages/MessageFactory.h"
#include "lwip/tcpip.h"

using rtps::StatefullReaderT;

template <class NetworkDriver>
void StatefullReaderT<NetworkDriver>::init(const BuiltInTopicData& attributes, NetworkDriver& driver){
    m_attributes = attributes;
    m_transport = &driver;
    m_packetInfo.srcPort = attributes.unicastLocator.port;
    sys_mutex_new(&mutex);
}

template <class NetworkDriver>
void StatefullReaderT<NetworkDriver>::newChange(ReaderCacheChange& cacheChange){

    if(m_callback == nullptr){
        return;
    }

    for(auto& proxy : m_proxies){
        if(proxy.remoteWriterGuid == cacheChange.writerGuid){
            if(proxy.expectedSN == cacheChange.sn){
                m_callback(m_callee, cacheChange);
                ++proxy.expectedSN;
                return;
            }
        }
    }
}

template <class NetworkDriver>
void StatefullReaderT<NetworkDriver>::registerCallback(ddsReaderCallback_fp cb, void* callee){
    m_callback = cb;
    m_callee = callee;
}

template <class NetworkDriver>
bool StatefullReaderT<NetworkDriver>::addNewMatchedWriter(const WriterProxy& newProxy){
    return m_proxies.add(newProxy);
}

template <class NetworkDriver>
void StatefullReaderT<NetworkDriver>::removeWriter(const Guid& guid){
    auto isElementToRemove=[&](const WriterProxy& proxy){
        return proxy.remoteWriterGuid == guid;
    };
    auto thunk=[](void* arg, const WriterProxy& value){return (*static_cast<decltype(isElementToRemove)*>(arg))(value);};

    m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
bool StatefullReaderT<NetworkDriver>::onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix){
    Lock lock(mutex);
    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;
    WriterProxy* writer = nullptr;
    for(auto& proxy : m_proxies){
        if(proxy.remoteWriterGuid.prefix == remotePrefix &&
           proxy.remoteWriterGuid.entityId == msg.writerId){
            writer = &proxy;
            break;
        }
    }

    if(writer == nullptr || msg.count.value <= writer->hbCount.value){
        return false;
    }

    writer->hbCount.value = msg.count.value;
    info.destAddr = writer->locator.getIp4Address();
    info.destPort = writer->locator.port;
    rtps::MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId, writer->getMissing(msg.firstSN,
                                     msg.lastSN), writer->getNextCount());

    m_transport->sendFunction(info);
    return true;
}
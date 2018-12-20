/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullReader.h"
#include "rtps/messages/MessageFactory.h"
#include "lwip/tcpip.h"


using rtps::StatefullReader;


void StatefullReader::init(Guid guid, UdpDriver& driver, Ip4Port_t sendPort){
    m_guid = guid;
    m_transport = &driver;
    m_packetInfo.srcPort = sendPort;
    sys_mutex_new(&mutex);
}

void StatefullReader::newChange(ReaderCacheChange& cacheChange){
    if(m_callback == nullptr){
        return;
    }

    for(auto& proxy : m_proxies){
        if(proxy.valid && proxy.writerProxy.remoteWriterGuid == cacheChange.writerGuid){
            if(cacheChange.sn == proxy.writerProxy.expectedSN){
                m_callback(m_callee, cacheChange);
                ++proxy.writerProxy.expectedSN;
                return;
            }else{
                printf("SequenceNumber is %u, expected: %u\n", cacheChange.sn.low, proxy.writerProxy.expectedSN);
                return;
            }
        }
    }
    printf("No matching WriterProxy found\n");
}

void StatefullReader::registerCallback(ddsReaderCallback_fp cb, void* callee){
    m_callback = cb;
    m_callee = callee;
}

rtps::WriterProxy* StatefullReader::createWriterProxy(Guid guid){
    for(auto& proxy : m_proxies){
        if(!proxy.valid){
            proxy.writerProxy.init(guid);
            proxy.valid = true;
            return &proxy.writerProxy;
        }
    }
    printf("StatefullReader: Not enough slots for additional writers");
    return nullptr;
}

void StatefullReader::removeWriter(const Guid& guid){
    for(auto& proxy : m_proxies){
        if(proxy.valid && proxy.writerProxy.remoteWriterGuid == guid){
            proxy.valid = false;
            return;
        }
    }
}

bool StatefullReader::onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePartGuidPrefix){
    Lock lock(mutex);
    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;
    WriterProxy* proxy = nullptr;
    for(auto& elem : m_proxies){
        if(elem.valid && elem.writerProxy.remoteWriterGuid.prefix == remotePartGuidPrefix &&
           elem.writerProxy.remoteWriterGuid.entityId == msg.writerId){
            proxy = &elem.writerProxy;
            break;
        }
    }

    if(proxy == nullptr || msg.count.value <= proxy->hbCount.value){
        return false;
    }

    proxy->hbCount.value = msg.count.value;
    info.destAddr = proxy->locator.getIp4Address();
    info.destPort = proxy->locator.port;
    rtps::MessageFactory::addHeader(info.buffer, m_guid.prefix);
    rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId, proxy->getMissing(msg.firstSN,
                                     msg.lastSN), proxy->getNextCount());

    m_transport->sendFunction(info);
    return true;
}
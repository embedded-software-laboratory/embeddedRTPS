/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullReader.h"
#include "rtps/messages/MessageFactory.h"


using rtps::StatefullReader;


StatefullReader::StatefullReader(Guid guid) : m_guid(guid){

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
            }else{
                return;
            }
        }
    }
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

void StatefullReader::onNewHeartbeat(SubmessageHeartbeat& msg, GuidPrefix_t remotePartGuidPrefix){
    WriterProxy* proxy = nullptr;
    for(auto& elem : m_proxies){
        if(elem.valid && elem.writerProxy.remoteWriterGuid.prefix == remotePartGuidPrefix &&
           elem.writerProxy.remoteWriterGuid.entityId == msg.writerId){
            proxy = &elem.writerProxy;
            break;
        }
    }

    if(proxy == nullptr){
        return;
    }

    UdpDriver::PacketInfo info;
    info.destAddr = proxy->locator.getIp4Address();
    info.destPort = proxy->locator.port;
    //TODO set sendport
    rtps::MessageFactory::addHeader(info.buffer, m_guid.prefix);
    rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId, proxy->getMissing(msg.firstSN,
                                     msg.lastSN), proxy->getNextCount());


    //TODO send
}
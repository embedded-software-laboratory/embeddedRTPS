/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessWriter.h"

#include "lwip/tcpip.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"
#include "rtps/utils/udpUtils.h"

namespace rtps{

    void StatelessWriter::init(TopicKind_t topicKind, ThreadPool* threadPool,
                          GuidPrefix_t guidPrefix, EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort){
        mp_threadPool = threadPool;
        m_transport = &driver;
        m_guidPrefix = guidPrefix;
        m_entityId = entityId;
        m_topicKind = topicKind;
        m_packetInfo.srcPort = sendPort;
        if (sys_mutex_new(&m_mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
        }
    }

    bool StatelessWriter::addNewMatchedReader(ReaderLocator loc){
        if(m_readerLocator.entityId != ENTITYID_UNKNOWN){
            return false;
        }
        m_readerLocator = loc;
        return true;
    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return m_lastChangeSequenceNumber;
    }

    const CacheChange* StatelessWriter::newChange(rtps::ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
        if(isIrrelevant(kind)){
            return &m_history.INVALID_CACHE_CHANGE;
        }

        ++m_lastChangeSequenceNumber;
        CacheChange change{};
        change.kind = kind;
        change.data.reserve(size);
        change.data.append(data, size);
        change.sequenceNumber = m_lastChangeSequenceNumber;


        Lock lock(m_mutex);
        auto result = m_history.addChange(std::move(change));
        if(mp_threadPool != nullptr){
            mp_threadPool->addWorkload(ThreadPool::Workload_t{this});
        }
        return result;
    }

    void StatelessWriter::unsentChangesReset() {
        Lock lock(m_mutex);
        m_history.resetSend();
        if(mp_threadPool != nullptr){
            mp_threadPool->addWorkload(ThreadPool::Workload_t{this});
        }
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::progress(){
        if(m_readerLocator.entityId == ENTITYID_UNKNOWN){ // TODO UNKNOWN might be okay. Detect not-set locator in another way
            return;
        }

        PacketInfo info;
        info.srcPort = m_packetInfo.srcPort;

        MessageFactory::addHeader(info.buffer, m_guidPrefix);
        MessageFactory::addSubMessageTimeStamp(info.buffer);

        {
            Lock lock(m_mutex);
            const CacheChange* next = m_history.getNextCacheChange();
            if(next == &m_history.INVALID_CACHE_CHANGE){
                printf("StatelessWriter: Couldn't get a new CacheChange\n");
                return;
            }
            MessageFactory::addSubMessageData(info.buffer, next->data, false, next->sequenceNumber, m_entityId,
                                              m_readerLocator.entityId); // TODO
        }

        // Just usable for IPv4
        const Locator& locator = m_readerLocator.locator;

        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t) locator.port;

        m_transport->sendFunction(info);
    }
}
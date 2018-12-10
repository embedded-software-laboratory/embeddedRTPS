/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatelessWriter.h"

#include "rtps/ThreadPool.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/utils/udpUtils.h"
#include "rtps/communication/UdpDriver.h"
#include "lwip/tcpip.h"

namespace rtps{

    void StatelessWriter::init(TopicKind_t topicKind, ThreadPool* threadPool,
                          GuidPrefix_t guidPrefix, EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort){
        mp_threadPool = threadPool;
        m_guidPrefix = guidPrefix;
        m_entityId = entityId;
        m_topicKind = topicKind;
        m_packetInfo.transport = &driver;
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
        // TODO lock
        m_packetInfo.buffer.reset();

        MessageFactory::addHeader(m_packetInfo.buffer, m_guidPrefix);
        MessageFactory::addSubMessageTimeStamp(m_packetInfo.buffer);

        {
            Lock lock(m_mutex);
            const CacheChange* next = m_history.getNextCacheChange();
            if(next == nullptr){
                return;
            }
            MessageFactory::addSubMessageData(m_packetInfo.buffer, next->data, false, next->sequenceNumber, m_entityId,
                                              m_readerLocator.entityId); // TODO
        }

        // Just usable for IPv4
        const Locator& locator = m_readerLocator.locator;

        IP4_ADDR((&m_packetInfo.destAddr), locator.address[12], locator.address[13], locator.address[14], locator.address[15]);
        m_packetInfo.destPort = (Ip4Port_t) locator.port;

        tcpip_callback(UdpDriver::sendFunctionJumppad, static_cast<void*>(&m_packetInfo));
    }
}
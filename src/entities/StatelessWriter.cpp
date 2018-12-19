/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <rtps/entities/ReaderProxy.h>
#include "rtps/entities/StatelessWriter.h"

#include "lwip/tcpip.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"
#include "rtps/utils/udpUtils.h"

namespace rtps{

    bool StatelessWriter::init(TopicKind_t topicKind, ThreadPool* threadPool,
                               GuidPrefix_t guidPrefix, EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort){
        if (sys_mutex_new(&m_mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
            return false;
        }
        mp_threadPool = threadPool;
        m_transport = &driver;
        m_guid.prefix = guidPrefix;
        m_guid.entityId = entityId;
        m_topicKind = topicKind;
        m_packetInfo.srcPort = sendPort;

        return true;
    }

    bool StatelessWriter::addNewMatchedReader(const ReaderProxy& newProxy){
        if(m_readerProxy.remoteReaderGuid.entityId != ENTITYID_UNKNOWN){
            return false;
        }
        m_readerProxy = newProxy;
        return true;
    }

    SequenceNumber_t StatelessWriter::getLastSequenceNumber() const{
        return m_lastChangeSequenceNumber;
    }

    const CacheChange* StatelessWriter::newChange(rtps::ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
        if(isIrrelevant(kind)){
            return nullptr; // TODO
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

        m_nextSequenceNumberToSend = m_history.getSeqNumMin();

        if(mp_threadPool != nullptr){
            mp_threadPool->addWorkload(ThreadPool::Workload_t{this});
        }
    }

    void StatelessWriter::onNewAckNack(const SubmessageAckNack& /*msg*/){
        // Too lazy to respond
    }

    bool StatelessWriter::isIrrelevant(ChangeKind_t kind) const{
        return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    }

    void StatelessWriter::progress(){
        if(m_readerProxy.remoteReaderGuid.entityId == ENTITYID_UNKNOWN){ // TODO UNKNOWN might be okay. Detect not-set locator in another way
            return;
        }

        PacketInfo info;
        info.srcPort = m_packetInfo.srcPort;

        MessageFactory::addHeader(info.buffer, m_guid.prefix);
        MessageFactory::addSubMessageTimeStamp(info.buffer);

        {
            Lock lock(m_mutex);
            const CacheChange* next = m_history.getChangeBySN(m_nextSequenceNumberToSend);
            if(next == nullptr){
                printf("StatelessWriter: Couldn't get a new CacheChange\n");
                return;
            }
            ++m_nextSequenceNumberToSend;
            MessageFactory::addSubMessageData(info.buffer, next->data, false, next->sequenceNumber, m_guid.entityId,
                                              m_readerProxy.remoteReaderGuid.entityId); // TODO
        }

        // Just usable for IPv4
        const Locator& locator = m_readerProxy.remoteLocator;

        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t) locator.port;

        m_transport->sendFunction(info);
    }
}
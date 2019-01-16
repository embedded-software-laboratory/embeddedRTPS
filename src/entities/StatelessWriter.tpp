/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <rtps/entities/ReaderProxy.h>

#include "lwip/tcpip.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/ThreadPool.h"
#include "rtps/utils/udpUtils.h"

using rtps::StatelessWriterT;
using rtps::SequenceNumber_t;
using rtps::CacheChange;

#define SLW_VERBOSE 0

#if SLW_VERBOSE
#include "rtps/utils/printutils.h"
#endif

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::init(BuiltInTopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, NetworkDriver& driver){
    if (sys_mutex_new(&m_mutex) != ERR_OK) {
        printf("Failed to create mutex \n");
        return false;
    }

    m_attributes = attributes;
    m_packetInfo.srcPort = attributes.unicastLocator.port;
    m_topicKind = topicKind;
    mp_threadPool = threadPool;
    m_transport = &driver;

    return true;
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::addNewMatchedReader(const ReaderProxy& newProxy){
#if SLW_VERBOSE
    printf("StatelessWriter[%s]: New reader added with id: ", &this->m_attributes.topicName[0]);
    printGuid(newProxy.remoteReaderGuid);
    printf("\n");
#endif

    if(m_readerProxy.remoteReaderGuid.entityId != ENTITYID_UNKNOWN){
        return false;
    }
    m_readerProxy = newProxy;
    return true;
}

template <typename NetworkDriver>
SequenceNumber_t StatelessWriterT<NetworkDriver>::getLastUsedSequenceNumber() const{
    return m_lastUsedChangeSequenceNumber;
}

template <typename NetworkDriver>
const CacheChange* StatelessWriterT<NetworkDriver>::newChange(rtps::ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
    if(isIrrelevant(kind)){
        return nullptr; // TODO
    }
    Lock lock(m_mutex);

    ++m_lastUsedChangeSequenceNumber;
    CacheChange change{};
    change.kind = kind;
    change.data.reserve(size);
    change.data.append(data, size);
    change.sequenceNumber = m_lastUsedChangeSequenceNumber;

    if(m_history.isFull()){
        SequenceNumber_t newMin = ++SequenceNumber_t{m_history.getSeqNumMin()};
        if(m_nextSequenceNumberToSend < newMin){
            m_nextSequenceNumberToSend = newMin; // Make sure we have the correct sn to send
        }
    }

    auto result = m_history.addChange(std::move(change));
    if(mp_threadPool != nullptr){
        mp_threadPool->addWorkload(ThreadPool::Workload_t{this});
    }

#if SLW_VERBOSE
    printf("StatelessWriter[%s]: Adding new data.\n", this->m_attributes.topicName);
#endif
    return result;
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::unsentChangesReset() {
    Lock lock(m_mutex);

    m_nextSequenceNumberToSend = m_history.getSeqNumMin();

    if(mp_threadPool != nullptr){
        mp_threadPool->addWorkload(ThreadPool::Workload_t{this});
    }
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::onNewAckNack(const SubmessageAckNack& /*msg*/){
    // Too lazy to respond
}

template <typename NetworkDriver>
bool StatelessWriterT<NetworkDriver>::isIrrelevant(ChangeKind_t kind) const{
    return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::progress(){
    if(m_readerProxy.remoteReaderGuid.entityId == ENTITYID_UNKNOWN){ // TODO UNKNOWN might be okay. Detect not-set locator in another way
        return;
    }

#if SLW_VERBOSE
    printf("StatelessWriter[%s]: Progess.\n", this->m_attributes.topicName);
#endif

    PacketInfo info;
    info.srcPort = m_packetInfo.srcPort;

    MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
    MessageFactory::addSubMessageTimeStamp(info.buffer);

    {
        Lock lock(m_mutex);
        const CacheChange* next = m_history.getChangeBySN(m_nextSequenceNumberToSend);
        if(next == nullptr){
#if SLW_VERBOSE
            printf("StatelessWriter[%s]: Couldn't get a new CacheChange with SN (%i,%i)\n",
                    &m_attributes.topicName[0], m_nextSequenceNumberToSend.high, m_nextSequenceNumberToSend.low);
#endif
            return;
        }else{
#if SLW_VERBOSE
            printf("StatelessWriter[%s]: Sending change with SN (%i,%i)\n",
                   &m_attributes.topicName[0], m_nextSequenceNumberToSend.high, m_nextSequenceNumberToSend.low);
#endif
        }
        ++m_nextSequenceNumberToSend;
        MessageFactory::addSubMessageData(info.buffer, next->data, false, next->sequenceNumber, m_attributes.endpointGuid.entityId,
                                          m_readerProxy.remoteReaderGuid.entityId); // TODO
    }

    // Just usable for IPv4
    const Locator& locator = m_readerProxy.remoteLocator;

    info.destAddr = locator.getIp4Address();
    info.destPort = (Ip4Port_t) locator.port;

    m_transport->sendFunction(info);
}
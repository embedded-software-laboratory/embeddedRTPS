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
#include "rtps/utils/Log.h"

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
#if SLW_VERBOSE
        Log::printLine("SFW:Failed to create mutex \n");
#endif
        return false;
    }

    m_attributes = attributes;
    m_packetInfo.srcPort = attributes.unicastLocator.port;
    m_topicKind = topicKind;
    mp_threadPool = threadPool;
    m_transport = &driver;

    return true;
}

template <class NetworkDriver>
bool StatelessWriterT<NetworkDriver>::addNewMatchedReader(const ReaderProxy& newProxy){
#if SLW_VERBOSE
    printf("StatefullWriter[%s]: New reader added with id: ", &this->m_attributes.topicName[0]);
    printGuid(newProxy.remoteReaderGuid);
    printf("\n");
#endif
    return m_proxies.add(newProxy);
}

template <class NetworkDriver>
void StatelessWriterT<NetworkDriver>::removeReader(const Guid& guid){
    auto isElementToRemove=[&](const ReaderProxy& proxy){
        return proxy.remoteReaderGuid == guid;
    };
    auto thunk=[](void* arg, const ReaderProxy& value){return (*static_cast<decltype(isElementToRemove)*>(arg))(value);};

    m_proxies.remove(thunk, &isElementToRemove);
}

template <typename NetworkDriver>
SequenceNumber_t StatelessWriterT<NetworkDriver>::getLastUsedSequenceNumber() const{
    return m_history.getSeqNumMax();
}

template <typename NetworkDriver>
const CacheChange* StatelessWriterT<NetworkDriver>::newChange(rtps::ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
    if(isIrrelevant(kind)){
        return nullptr;
    }
    Lock lock(m_mutex);

    if(m_history.isFull()){
        SequenceNumber_t newMin = ++SequenceNumber_t(m_history.getSeqNumMin());
        if(m_nextSequenceNumberToSend < newMin){
            m_nextSequenceNumberToSend = newMin; // Make sure we have the correct sn to send
        }
    }

    auto result = m_history.addChange(data, size);
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
    // Right now we only allow alive changes
    //return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    return kind != ChangeKind_t::ALIVE;
}

template <typename NetworkDriver>
void StatelessWriterT<NetworkDriver>::progress(){
    // TODO smarter packaging e.g. by creating MessageStruct and serialize after adjusting values
    for(const auto& proxy : m_proxies){

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
                                              proxy.remoteReaderGuid.entityId); // TODO
        }

        // Just usable for IPv4
        const Locator& locator = proxy.remoteLocator;

        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t) locator.port;

        m_transport->sendFunction(info);

    }
}

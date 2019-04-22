/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/entities/StatefullWriter.h"

#include "rtps/messages/MessageFactory.h"
#include <cstring>

using rtps::StatefullWriterT;

#define SFW_VERBOSE 0

#if SFW_VERBOSE
#include "rtps/utils/printutils.h"
#endif


template <class NetworkDriver>
StatefullWriterT<NetworkDriver>::~StatefullWriterT(){
    m_running = false;
    sys_msleep(10); // Required for tests/ Join currently not available
}

template <class NetworkDriver>
bool StatefullWriterT<NetworkDriver>::init(TopicData attributes, TopicKind_t topicKind, ThreadPool* /*threadPool*/, NetworkDriver& driver){
    if (sys_mutex_new(&m_mutex) != ERR_OK) {
#if SFW_VERBOSE
        printf("StatefullWriter: Failed to create mutex.\n");
#endif
        return false;
    }

    m_transport = &driver;
    m_attributes = attributes;
    m_topicKind = topicKind;
    m_packetInfo.srcPort = attributes.unicastLocator.port;
    if(m_attributes.endpointGuid.entityId == ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER){
    	m_heartbeatThread = sys_thread_new("HBThreadPub", hbFunctionJumppad, this, Config::HEARTBEAT_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
    }else if(m_attributes.endpointGuid.entityId == ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER){
    	m_heartbeatThread = sys_thread_new("HBThreadSub", hbFunctionJumppad, this, Config::HEARTBEAT_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
    }else{
    	 m_heartbeatThread = sys_thread_new("HBThread", hbFunctionJumppad, this, Config::HEARTBEAT_STACKSIZE, Config::THREAD_POOL_WRITER_PRIO);
    }

    return true;
}

template <class NetworkDriver>
bool StatefullWriterT<NetworkDriver>::addNewMatchedReader(const ReaderProxy& newProxy){
#if SFW_VERBOSE
    printf("StatefullWriter[%s]: New reader added with id: ", &this->m_attributes.topicName[0]);
    printGuid(newProxy.remoteReaderGuid);
    printf("\n");
#endif
    return m_proxies.add(newProxy);
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::removeReader(const Guid& guid){
    auto isElementToRemove=[&](const ReaderProxy& proxy){
        return proxy.remoteReaderGuid == guid;
    };
    auto thunk=[](void* arg, const ReaderProxy& value){return (*static_cast<decltype(isElementToRemove)*>(arg))(value);};

    m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::progress(){

}

template <class NetworkDriver>
const rtps::CacheChange* StatefullWriterT<NetworkDriver>::newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) {
    if (isIrrelevant(kind)) {
        return nullptr;
    }

    Lock lock{m_mutex};

    // Right now we drop elements anyway because we cannot detect non-responding readers yet.
    // if(history.isFull()){
    //     return nullptr;
    // }
#if SFW_VERBOSE
    printf("StatefullWriter[%s]: Adding new data.\n", this->m_attributes.topicName);
#endif
    return m_history.addChange(data, size);
}

template <typename NetworkDriver>
bool StatefullWriterT<NetworkDriver>::isIrrelevant(ChangeKind_t kind) const{
    // Right now we only allow alive changes
    //return kind == ChangeKind_t::INVALID || (m_topicKind == TopicKind_t::NO_KEY && kind != ChangeKind_t::ALIVE);
    return kind != ChangeKind_t::ALIVE;
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::unsentChangesReset(){
    // Don't see a reason why this might be needed for a reliable writer
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::onNewAckNack(const SubmessageAckNack& msg){
	//Lock lock{m_mutex};
    // Search for reader
    ReaderProxy* reader = nullptr;
    for(auto& proxy : m_proxies){
        if(proxy.remoteReaderGuid.entityId == msg.readerId){
            reader = &proxy;
            break;
        }
    }

    if(reader == nullptr){
#if SFW_VERBOSE
        printf("StatefullWriter[%s]: No proxy found with id: ", &this->m_attributes.topicName[0]);
        printEntityId(msg.readerId);
        printf(" Dropping acknack.\n");
#endif
        return;
    }

    if(msg.count.value <= reader->ackNackCount.value){
#if SFW_VERBOSE
        printf("StatefullWriter[%s]: Count too small. Dropping acknack.\n", &this->m_attributes.topicName[0]);
#endif
        return;
    }

    reader->ackNackCount = msg.count;

    // Send missing packets
    SequenceNumber_t nextSN = msg.readerSNState.base;
#if SFW_VERBOSE
    if(nextSN.low == 0 && nextSN.high == 0){
        printf("StatefullWriter[%s]: Received preemptive acknack. Ignored.\n", &this->m_attributes.topicName[0]);
    }else{
        printf("StatefullWriter[%s]: Received non-preemptive acknack.\n", &this->m_attributes.topicName[0]);
    }
#endif
    for(uint32_t i=0; i < msg.readerSNState.numBits; ++i, ++nextSN){
        if(msg.readerSNState.isSet(i)){
#if SFW_VERBOSE
            printf("StatefullWriter[%s]: Send Packet on acknack.\n", this->m_attributes.topicName);
#endif
            sendData(*reader, nextSN);
        }
    }
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::sendData(const ReaderProxy &reader, const SequenceNumber_t &sn){
    SequenceNumber_t max_sn;
    {
        Lock lock(m_mutex);
        max_sn = m_history.getSeqNumMax();
    }

    // send the missing one and all following
    for(SequenceNumber_t s = sn; s <= max_sn; ++s){

        PacketInfo info;
        info.srcPort = m_packetInfo.srcPort;

        MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
        MessageFactory::addSubMessageTimeStamp(info.buffer);

        // Just usable for IPv4
        const Locator& locator = reader.remoteLocator;

        info.destAddr = locator.getIp4Address();
        info.destPort = (Ip4Port_t) locator.port;

        {
            Lock lock(m_mutex);
            const CacheChange* next = m_history.getChangeBySN(sn);
            if(next == nullptr){
    #if SFW_VERBOSE
                printf("StatefullWriter[%s]: Couldn't get a CacheChange with SN (%i,%u)\n", &this->m_attributes.topicName[0], sn.high, sn.low);
    #endif
                continue;
            }
            MessageFactory::addSubMessageData(info.buffer, next->data, false, next->sequenceNumber, m_attributes.endpointGuid.entityId,
                                              reader.remoteReaderGuid.entityId);
        }

        m_transport->sendPacket(info);

    }

}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::hbFunctionJumppad(void* thisPointer){
    auto* writer = static_cast<StatefullWriterT<NetworkDriver>*>(thisPointer);
    writer->sendHeartBeatLoop();
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::sendHeartBeatLoop(){
    while(m_running){
        sendHeartBeat();
        sys_msleep(m_heartbeatPeriodMs);
    }
}

template <class NetworkDriver>
void StatefullWriterT<NetworkDriver>::sendHeartBeat() {
    if(m_proxies.isEmpty()){
#if SFW_VERBOSE
        printf("StatefullWriter[%s]: Skipping heartbeat. No proxies.\n", this->m_attributes.topicName);
#endif
        return;
    }

    for(auto& proxy : m_proxies) {

        PacketInfo info;
        info.srcPort = m_packetInfo.srcPort;

        SequenceNumber_t firstSN;
        SequenceNumber_t lastSN;
        MessageFactory::addHeader(info.buffer, m_attributes.endpointGuid.prefix);
        {
            Lock lock(m_mutex);
            firstSN = m_history.getSeqNumMin();
            lastSN = m_history.getSeqNumMax();
        }
        if (firstSN == SEQUENCENUMBER_UNKNOWN || lastSN == SEQUENCENUMBER_UNKNOWN) {
#if SFW_VERBOSE
            if(strlen(&this->m_attributes.typeName[0]) != 0){
                printf("StatefullWriter[%s]: Skipping heartbeat. No data.\n", this->m_attributes.topicName);
            }
#endif
            return;
        }

        MessageFactory::addHeartbeat(info.buffer, m_attributes.endpointGuid.entityId,
                                     proxy.remoteReaderGuid.entityId, firstSN, lastSN, m_hbCount);

        info.destAddr = proxy.remoteLocator.getIp4Address();
        info.destPort = proxy.remoteLocator.port;

        m_transport->sendPacket(info);
    }
    m_hbCount.value++;
}

#undef SFW_VERBOSE

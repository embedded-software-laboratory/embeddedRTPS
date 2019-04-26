/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULLWRITER_H
#define RTPS_STATEFULLWRITER_H

#include "rtps/entities/Writer.h"
#include "rtps/entities/ReaderProxy.h"
#include "rtps/storages/MemoryPool.h"
#include "rtps/storages/SimpleHistoryCache.h"

namespace rtps{

    template <class NetworkDriver>
    class StatefullWriterT final : public Writer{
    public:

        ~StatefullWriterT() override;
        bool init(TopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, NetworkDriver& driver);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
        void removeReader(const Guid& guid) override;
        //! Executes required steps like sending packets. Intended to be called by worker threads
        void progress() override;
        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) override;
        void unsentChangesReset() override;
        void onNewAckNack(const SubmessageAckNack& msg) override;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        PacketInfo m_packetInfo;
        NetworkDriver* m_transport;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_nextSequenceNumberToSend = {0, 1};
        SimpleHistoryCache m_history;
        sys_thread_t m_heartbeatThread;
        Count_t m_hbCount{1};

        bool m_running = true;

        MemoryPool<ReaderProxy, Config::NUM_READER_PROXIES_PER_WRITER> m_proxies;

        void sendData(const ReaderProxy &reader, const SequenceNumber_t &sn);
        static void hbFunctionJumppad(void* thisPointer);
        void sendHeartBeatLoop();
        void sendHeartBeat();
        bool isIrrelevant(ChangeKind_t kind) const;
    };

    using StatefullWriter = StatefullWriterT<UdpDriver>;
}

#include "StatefullWriter.tpp"

#endif //RTPS_STATEFULLWRITER_H

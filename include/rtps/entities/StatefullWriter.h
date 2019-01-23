/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULLWRITER_H
#define RTPS_STATEFULLWRITER_H

#include "rtps/entities/Writer.h"
#include "rtps/entities/ReaderProxy.h"

namespace rtps{

    template <class NetworkDriver>
    class StatefullWriterT final : public Writer{
    public:
        bool init(BuiltInTopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, UdpDriver& driver);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
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
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        HistoryCache m_history;
        static const uint32_t m_heartbeatPeriodMs = 4000; // TODO
        sys_thread_t m_heartbeatThread;
        Count_t m_hbCount{1};

        uint32_t m_proxySlotUsedBitMap = 0;
        static_assert(sizeof(m_proxySlotUsedBitMap)*8 >= Config::NUM_READER_PROXIES_PER_WRITER,
                      "StatefullWriter: Bitmap too small");
        ReaderProxy m_proxies[Config::NUM_READER_PROXIES_PER_WRITER];

        void sendData(const ReaderProxy &reader, const SequenceNumber_t &sn);
        static void hbFunctionJumppad(void* thisPointer);
        void sendHeartBeat();
    };

    using StatefullWriter = StatefullWriterT<UdpDriver>;
}

#include "StatefullWriter.tpp"

#endif //RTPS_STATEFULLWRITER_H

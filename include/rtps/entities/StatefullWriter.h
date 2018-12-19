/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULLWRITER_H
#define RTPS_STATEFULLWRITER_H

#include "rtps/entities/Writer.h"
#include "rtps/entities/ReaderProxy.h"

namespace rtps{

    class StatefullWriter final : public Writer{
    public:
        bool init(TopicKind_t topicKind, ThreadPool* threadPool, GuidPrefix_t guidPrefix,
                  EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
        //! Executes required steps like sending packets. Intended to be called by worker threads
        void progress() override;
        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) override;
        void unsentChangesReset() override;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        PacketInfo m_packetInfo;
        UdpDriver* m_transport;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        HistoryCache m_history;

        uint32_t m_proxySlotUsedBitMap = 0;
        static_assert(sizeof(m_proxySlotUsedBitMap)*8 >= Config::NUM_READER_PROXIES_PER_WRITER,
                      "StatefullWriter: Bitmap too small");
        ReaderProxy m_proxies[Config::NUM_READER_PROXIES_PER_WRITER];
    };

}

#endif //RTPS_STATEFULLWRITER_H

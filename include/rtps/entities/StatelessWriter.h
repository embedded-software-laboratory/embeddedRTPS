/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "lwip/sys.h"
#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/MemoryPool.h"
#include "rtps/storages/SimpleHistoryCache.h"

namespace rtps {

    struct PBufWrapper;

    template <typename NetworkDriver>
    class StatelessWriterT : public Writer{
    public:

        bool init(TopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, NetworkDriver& driver);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
        void removeReader(const Guid& guid) override;
        void progress() override;
        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) override;
        void unsentChangesReset() override;
        void onNewAckNack(const SubmessageAckNack& msg) override;

        SequenceNumber_t getLastUsedSequenceNumber() const;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        PacketInfo m_packetInfo;
        NetworkDriver* m_transport;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_nextSequenceNumberToSend = {0, 1};
        SimpleHistoryCache m_history;

        MemoryPool<ReaderProxy, Config::NUM_READER_PROXIES_PER_WRITER> m_proxies;

        bool isIrrelevant(ChangeKind_t kind) const;

    };

    using StatelessWriter = StatelessWriterT<UdpDriver>;

}

#include "StatelessWriter.tpp"

#endif //RTPS_RTPSWRITER_H

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

namespace rtps {

    struct PBufWrapper;

    template <typename NetworkDriver>
    class StatelessWriterT : public Writer{
    public:

        bool init(BuiltInTopicData attributes, TopicKind_t topicKind, ThreadPool* threadPool, NetworkDriver& driver);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
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
        SequenceNumber_t m_lastUsedChangeSequenceNumber = {0, 0};
        SequenceNumber_t m_nextSequenceNumberToSend = {0, 1};
        HistoryCache m_history;
        ReaderProxy m_readerProxy{};

        bool isIrrelevant(ChangeKind_t kind) const;

    };

    using StatelessWriter = StatelessWriterT<UdpDriver>;

}

#include "../../src/entities/StatelessWriter.tpp"

#endif //RTPS_RTPSWRITER_H

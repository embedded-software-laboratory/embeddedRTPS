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

    class StatelessWriter : public Writer{
    public:

        bool init(TopicKind_t topicKind, ThreadPool* threadPool, GuidPrefix_t guidPrefix,
                  EntityId_t entityId, UdpDriver& driver, Ip4Port_t sendPort);

        bool addNewMatchedReader(const ReaderProxy& newProxy) override;
        void progress() override;
        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) override;
        void unsentChangesReset() override;

        SequenceNumber_t getLastSequenceNumber() const;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        PacketInfo m_packetInfo;
        UdpDriver* m_transport;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        SequenceNumber_t m_nextSequenceNumberToSend = {0, 1};
        HistoryCache m_history;
        ReaderProxy m_readerProxy{};

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

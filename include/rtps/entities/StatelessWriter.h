/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/entities/Writer.h"
#include "lwip/sys.h"

namespace rtps {

    struct PBufWrapper;

    class StatelessWriter : public Writer{
    public:

        void init(TopicKind_t topicKind, ThreadPool* threadPool,
                  GuidPrefix_t guidPrefix, EntityId_t entityId, Ip4Port_t sendPort);

        bool addNewMatchedReader(ReaderLocator loc) override;
        bool createMessageCallback(ThreadPool::PacketInfo& packetInfo) override;
        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, DataSize_t size) override;
        void unsentChangesReset() override;

        SequenceNumber_t getLastSequenceNumber() const;



    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        GuidPrefix_t m_guidPrefix = GUIDPREFIX_UNKNOWN;
        EntityId_t m_entityId = ENTITYID_UNKNOWN;
        Ip4Port_t m_sendPort = 0;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        HistoryCache m_history;
        ReaderLocator m_readerLocator{};

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/entities/Writer.h"
#include "lwip/sys.h"

namespace rtps {

    struct PBufWrapper;

    class StatelessWriter : public Writer{
    public:

        void init(TopicKind_t topicKind, Locator_t locator, ThreadPool* threadPool,
                  GuidPrefix_t guidPrefix, EntityId_t entityId, participantId_t participantId);

        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size) override;

        void unsentChangesReset() override;

        SequenceNumber_t getLastSequenceNumber() const;

        void createMessageCallback(ThreadPool::PacketInfo& packetInfo) override;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        GuidPrefix_t m_guidPrefix = GUIDPREFIX_UNKNOWN;
        EntityId_t m_entityId = ENTITYID_UNKNOWN;
        ip4_port_t m_sendPort = 0;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        HistoryCache m_history;
        Locator_t m_locator{};

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

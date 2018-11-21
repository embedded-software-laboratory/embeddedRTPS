/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/HistoryCache.h"
#include "lwip/sys.h"

namespace rtps {

    class ThreadPool;
    class PBufWrapper;

    class StatelessWriter : public Writer{
    public:

        void init(TopicKind_t topicKind, Locator_t locator, ThreadPool* threadPool,
                  GuidPrefix_t guidPrefix, EntityId_t entityId);

        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size) override;

        void unsentChangesReset() override;

        SequenceNumber_t getLastSequenceNumber() const;

        void createMessageCallback(PBufWrapper& buffer) override;

    private:
        sys_mutex_t m_mutex;
        ThreadPool* mp_threadPool = nullptr;

        GuidPrefix_t m_guidPrefix = GUIDPREFIX_UNKNOWN;
        EntityId_t m_entityId = ENTITYID_UNKNOWN;

        TopicKind_t m_topicKind = TopicKind_t::NO_KEY;
        SequenceNumber_t m_lastChangeSequenceNumber = {0, 0};
        HistoryCache m_history;
        Locator_t m_locator{};

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

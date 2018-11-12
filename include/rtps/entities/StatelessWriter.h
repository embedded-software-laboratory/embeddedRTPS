/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/ThreadPool.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/HistoryCache.h"

namespace rtps {

class StatelessWriter : public Writer{
public:

    StatelessWriter(TopicKind_t topicKind, Locator_t locator, ThreadPool* threadPool);

    const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size);

    void removeChange(const CacheChange* change);

    void unsentChangesReset();

    SequenceNumber_t getLastSequenceNumber() const;

    void createMessageCallback(PBufWrapper& buffer) override;

private:
    sys_mutex_t mutex;
    ThreadPool* threadPool;

    const TopicKind_t topicKind;
    const ReliabilityKind_t reliability = ReliabilityKind_t::BEST_EFFORT;
    SequenceNumber_t lastChangeSequenceNumber = {0, 0};
    HistoryCache history;
    const Locator_t locator;
    const EntityId_t writerId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;

    bool isIrrelevant(ChangeKind_t kind) const;

};

}

#endif //RTPS_RTPSWRITER_H

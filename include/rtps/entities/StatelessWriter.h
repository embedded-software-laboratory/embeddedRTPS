/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/types.h"
#include "rtps/config.h"
#include "rtps/entities/Writer.h"
#include "rtps/storages/PBufWrapper.h"
#include "rtps/storages/HistoryCache.h"

namespace rtps {

class StatelessWriter : public Writer{
    public:
        HistoryCache history;

        StatelessWriter(TopicKind_t topicKind);

        const CacheChange* newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size);

        SequenceNumber_t getLastSequenceNumber() const;

        bool addReaderLocator(Locator_t& loc);

        bool removeReaderLocator(Locator_t& loc);

        void createMessageCallback(PBufWrapper& buffer) override;

    private:
        const TopicKind_t topicKind;
        const ReliabilityKind_t reliability = ReliabilityKind_t::BEST_EFFORT;
        SequenceNumber_t lastChangeSequenceNumber = {0, 0};
        sys_mutex_t mutex;

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

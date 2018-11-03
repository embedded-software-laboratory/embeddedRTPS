/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RTPSWRITER_H
#define RTPS_RTPSWRITER_H

#include "rtps/types.h"

namespace rtps {

    struct CacheChange{
        ChangeKind_t kind = ChangeKind_t::INVALID;
        uint8_t* data = nullptr;
        uint16_t size = 0;
    };

    class StatelessWriter {
    public:
        StatelessWriter(TopicKind_t topicKind);

        SequenceNumber_t getLastSequenceNumber() const;
        CacheChange newChange(ChangeKind_t kind, uint8_t* data, data_size_t size);

    private:
        TopicKind_t topicKind;
        SequenceNumber_t lastChangeSequenceNumber = {0, 0};
        CacheChange change;

        bool isIrrelevant(ChangeKind_t kind) const;

    };

}

#endif //RTPS_RTPSWRITER_H

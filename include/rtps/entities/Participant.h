/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PARTICIPANT_H
#define RTPS_PARTICIPANT_H

#include "rtps/types.h"

namespace rtps{

    class Participant{
    public:
        const GuidPrefix_t guidPrefix;
        const participantId_t participantId;

        explicit Participant(GuidPrefix_t guidPrefix, participantId_t participantId)
            : guidPrefix(guidPrefix), participantId(participantId){};
    private:

    };
}

#endif //RTPS_PARTICIPANT_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_DOMAIN_H
#define RTPS_DOMAIN_H

#include "rtps/config.h"
#include "rtps/entities/Participant.h"
#include "rtps/storages/PBufWrapper.h"

namespace rtps{
    class Domain{
    public:
        void receiveCallback(PBufWrapper buffer);

        Participant* createParticipant();

    private:
        //std::array<Participant, Config::MAX_NUM_PARTICIPANTS> m_participants{};
    };
}

#endif //RTPS_DOMAIN_H

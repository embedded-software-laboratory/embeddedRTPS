/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_WRITER_H
#define RTPS_WRITER_H

#include "rtps/storages/PBufWrapper.h"
namespace rtps{

    class Writer{
    private:
    public:
        virtual void createMessageCallback(PBufWrapper& buffer) = 0;
    };
}

#endif //RTPS_WRITER_H

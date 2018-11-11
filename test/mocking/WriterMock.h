/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_WRITERMOCK_H
#define RTPS_WRITERMOCK_H

#include "rtps/entities/Writer.h"

#include <gmock/gmock.h>


class WriterMock : public rtps::Writer{
public:
    MOCK_METHOD1(createMessageCallback, void(rtps::PBufWrapper&));

};
#endif //RTPS_WRITERMOCK_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_READERMOCK_H
#define RTPS_READERMOCK_H

#include <gmock/gmock.h>

#include "rtps/entities/Reader.h"
#include "rtps/storages/HistoryCache.h"

using rtps::Reader;

class ReaderMock final: public Reader{
public:

    explicit ReaderMock(rtps::EntityId_t id) : Reader(id){}
    ~ReaderMock() override = default;

    MOCK_METHOD3(newChange, void(rtps::ChangeKind_t, const uint8_t*, rtps::data_size_t));

    MOCK_METHOD0(registerCallback, void());

};

#endif //RTPS_READERMOCK_H

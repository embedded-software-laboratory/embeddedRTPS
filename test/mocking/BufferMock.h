/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_BUFFERMOCK_H
#define RTPS_BUFFERMOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "rtps/types.h"

#include <vector>
#include <cstdint>
#include <array>

struct BufferMock{
    std::vector<uint8_t> buffer;
    rtps::data_size_t reserved = 0;

    BufferMock() = default;
    explicit BufferMock(uint16_t){
    };

    bool isValid() const{
        return true;
    }

    void assertEnoughSpace(rtps::data_size_t length){
        ASSERT_LE(length, reserved);
    }
    bool append(const uint8_t *const data, rtps::data_size_t length){
        assertEnoughSpace(length);
        if(reserved < length){
            return false;
        }
        for(auto i=0; i < length; ++i){
            buffer.push_back(data[i]);
        }
        reserved -= length;
        return true;
    }

    bool append(BufferMock&& other){
        buffer.insert(std::end(buffer), std::begin(other.buffer), std::end(other.buffer));
        reserved = other.reserved;
        other.buffer.clear();
    }

    bool reserve(rtps::data_size_t length){
        reserved = length;
        return true;
    }

    uint8_t operator[](uint16_t idx) const{
        return buffer[idx];
    }

    std::vector<uint8_t>::iterator getIterator(int idx){
        return buffer.begin()+idx;
    }

    template <size_t N>
    void hasElementsAt(std::array<uint8_t, N> elements, int beginIdx){
        ASSERT_GE(buffer.size() - beginIdx, N);
        for(size_t i =0; i < N; ++i){
            EXPECT_EQ(buffer[beginIdx + i], elements[i]) << "Failed at index " << i << '\n';
        }

    }

};
#endif //RTPS_BUFFERMOCK_H

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_PBUFWRAPPER_H
#define RTPS_PBUFWRAPPER_H

#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "rtps/types.h"

/**
 * This Wrapper handles the lifetime of a pbuf element. Allocates it
 * when constructed and frees is again when running out of scope.
 */

namespace rtps {

    struct PBufWrapper {
        struct PBufPosition{
            pbuf* element;
            data_size_t offset;
        };

        pbuf *firstElement = nullptr;

        ip4_addr_t addr{};
        ip4_port_t port = 0;

        PBufWrapper() = default;
        explicit PBufWrapper(data_size_t length);

        PBufWrapper &operator=(PBufWrapper &&other) noexcept;

        ~PBufWrapper();

        bool isValid() const;

        bool append(const uint8_t *const data, data_size_t length);

        bool reserve(data_size_t length);


    private:


        constexpr static pbuf_layer m_layer = PBUF_TRANSPORT;
        constexpr static pbuf_type m_type = PBUF_POOL;

        data_size_t m_usedMemory = 0;
        PBufPosition m_nextEmptyByte{nullptr, 0};


        data_size_t spaceLeft() const;
        bool increaseSize(uint16_t length);
    };

}

#endif //RTPS_PBUFWRAPPER_H

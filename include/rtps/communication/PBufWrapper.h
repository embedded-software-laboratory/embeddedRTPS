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
        pbuf *firstElement = nullptr;

        ip4_addr_t addr{};
        ip4_port_t port = 0;

        PBufWrapper();

        PBufWrapper(pbuf_layer layer, u16_t length, pbuf_type type);

        PBufWrapper &operator=(PBufWrapper &&other);

        ~PBufWrapper();

        bool isValid();

        bool fillBuffer(const uint8_t *const data, uint16_t length);

    };

}

#endif //RTPS_PBUFWRAPPER_H

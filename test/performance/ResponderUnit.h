/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_RESPONDERUNIT_H
#define RTPS_RESPONDERUNIT_H

#include "rtps/entities/Domain.h"

#include <vector>

namespace rtps {
    namespace tests {

        class ResponderUnit {
        public:
            explicit ResponderUnit(uint32_t maxMsgSizeInBytes);

            void run();

        private:
            Domain m_domain;
            Writer* mp_dataWriter;
            Reader* mp_dataReader;

            std::vector<uint8_t> m_buffer;

            void prepareRTPS();
            static void responderJumppad(void* vp_writer, rtps::ReaderCacheChange& cacheChange);
            void responderCallback(rtps::ReaderCacheChange& cacheChange);
        };
    }
}

#endif //RTPS_RESPONDERUNIT_H

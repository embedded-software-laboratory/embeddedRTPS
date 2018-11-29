/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATELESSREADER_H
#define RTPS_STATELESSREADER_H

#include "rtps/entities/Reader.h"
#include "rtps/storages/HistoryCache.h"

namespace rtps{
    class StatelessReader final: public Reader{
    public:
        void newChange(ChangeKind_t kind, const uint8_t* data, data_size_t size) override;
        void registerCallback(ddsReaderCallback_fp cb) override;

    private:
        HistoryCache historyCache;
        ddsReaderCallback_fp m_callback = nullptr;
    };

}

#endif //RTPS_STATELESSREADER_H

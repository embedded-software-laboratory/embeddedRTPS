/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULLREADER_H
#define RTPS_STATEFULLREADER_H

#include "rtps/entities/Reader.h"
#include "rtps/config.h"
#include "rtps/entities/WriterProxy.h"

namespace rtps{

    struct SubmessageHeartbeat;

    class StatefullReader final: public Reader{
    public:

        explicit StatefullReader(Guid guid);
        void newChange(ReaderCacheChange& cacheChange) override;
        void registerCallback(ddsReaderCallback_fp cb, void* callee) override;
        WriterProxy* createWriterProxy(Guid guid);
        void removeWriter(const Guid& guid);
        void onNewHeartbeat(SubmessageHeartbeat& msg, GuidPrefix_t remotePrefix);

    private:
        struct Element{
            bool valid = false;
            WriterProxy writerProxy;
        };
        Guid m_guid;
        Ip4Port_t m_sendPort;
        Element m_proxies[Config::NUM_WRITER_PROXIES_PER_READER];
        ddsReaderCallback_fp m_callback = nullptr;
        void* m_callee = nullptr;

    };

}
#endif //RTPS_STATEFULLREADER_H

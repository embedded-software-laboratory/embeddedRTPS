/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULLREADER_H
#define RTPS_STATEFULLREADER_H

#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/config.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/WriterProxy.h"

namespace rtps{
    struct SubmessageHeartbeat;

    class StatefullReader final: public Reader{
    public:

        void init(Guid guid, UdpDriver& driver, Ip4Port_t sendPort);
        void newChange(ReaderCacheChange& cacheChange) override;
        void registerCallback(ddsReaderCallback_fp cb, void* callee) override;
        WriterProxy* createWriterProxy(Guid guid);
        void removeWriter(const Guid& guid);
        void onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix) override;

    private:
        struct Element{
            bool valid = false;
            WriterProxy writerProxy;
        };
        PacketInfo m_packetInfo;
        UdpDriver* m_transport;
        sys_mutex_t mutex;
        Element m_proxies[Config::NUM_WRITER_PROXIES_PER_READER];
        ddsReaderCallback_fp m_callback = nullptr;
        void* m_callee = nullptr;

    };

}
#endif //RTPS_STATEFULLREADER_H

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
#include "rtps/storages/MemoryPool.h"

namespace rtps{
    struct SubmessageHeartbeat;

    template <class NetworkDriver=UdpDriver>
    class StatefullReaderT : public Reader{
    public:

        void init(const BuiltInTopicData& attributes, NetworkDriver& driver);
        void newChange(ReaderCacheChange& cacheChange) override;
        void registerCallback(ddsReaderCallback_fp cb, void* callee) override;
        bool addNewMatchedWriter(const WriterProxy& newProxy) override;
        void removeWriter(const Guid& guid);
        bool onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix) override;

    private:
        PacketInfo m_packetInfo; // TODO intended for reuse but buffer not used as such
        NetworkDriver* m_transport;
        MemoryPool<WriterProxy, Config::NUM_WRITER_PROXIES_PER_READER> m_proxies;

        ddsReaderCallback_fp m_callback = nullptr;
        void* m_callee = nullptr;
        sys_mutex_t m_mutex;

    };

    using StatefullReader = StatefullReaderT<UdpDriver>;

}

#include "StatefullReader.tpp"


#endif //RTPS_STATEFULLREADER_H

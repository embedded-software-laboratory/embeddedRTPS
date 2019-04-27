/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_STATEFULREADER_H
#define RTPS_STATEFULREADER_H

#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/config.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/WriterProxy.h"
#include "rtps/storages/MemoryPool.h"

namespace rtps{
    struct SubmessageHeartbeat;

    template <class NetworkDriver>
    class StatefulReaderT final: public Reader{
    public:
        ~StatefulReaderT() override;
        void init(const TopicData& attributes, NetworkDriver& driver);
        void newChange(const ReaderCacheChange& cacheChange) override;
        void registerCallback(ddsReaderCallback_fp cb, void* callee) override;
        bool addNewMatchedWriter(const WriterProxy& newProxy) override;
        void removeWriter(const Guid& guid) override;
        bool onNewHeartbeat(const SubmessageHeartbeat& msg, const GuidPrefix_t& remotePrefix) override;

    private:
        PacketInfo m_packetInfo; // TODO intended for reuse but buffer not used as such
        NetworkDriver* m_transport;
        MemoryPool<WriterProxy, Config::NUM_WRITER_PROXIES_PER_READER> m_proxies;

        ddsReaderCallback_fp m_callback = nullptr;
        void* m_callee = nullptr;
        sys_mutex_t m_mutex;

    };

    using StatefulReader = StatefulReaderT<UdpDriver>;

}

#include "StatefulReader.tpp"


#endif //RTPS_STATEFULREADER_H

/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#ifndef RTPS_STATEFULREADER_H
#define RTPS_STATEFULREADER_H

#include "lwip/sys.h"
#include "rtps/communication/UdpDriver.h"
#include "rtps/config.h"
#include "rtps/entities/Reader.h"
#include "rtps/entities/WriterProxy.h"
#include "rtps/storages/MemoryPool.h"

namespace rtps {
struct SubmessageHeartbeat;

template <class NetworkDriver> class StatefulReaderT final : public Reader {
public:
  ~StatefulReaderT() override;
  bool init(const TopicData &attributes, NetworkDriver &driver);
  void newChange(const ReaderCacheChange &cacheChange) override;
  bool addNewMatchedWriter(const WriterProxy &newProxy) override;
  bool onNewHeartbeat(const SubmessageHeartbeat &msg,
                      const GuidPrefix_t &remotePrefix) override;
  bool onNewGapMessage(const SubmessageGap &msg,
                       const GuidPrefix_t &remotePrefix) override;

private:
  Ip4Port_t m_srcPort; // TODO intended for reuse but buffer not used as such
  NetworkDriver *m_transport;
};

using StatefulReader = StatefulReaderT<UdpDriver>;

} // namespace rtps

#include "StatefulReader.tpp"

#endif // RTPS_STATEFULREADER_H

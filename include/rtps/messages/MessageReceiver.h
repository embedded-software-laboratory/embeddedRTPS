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

#ifndef RTPS_MESSAGERECEIVER_H
#define RTPS_MESSAGERECEIVER_H

#include "rtps/common/types.h"
#include "rtps/config.h"
#include "rtps/discovery/BuiltInEndpoints.h"
#include <cstdint>

namespace rtps {
class Reader;
class Writer;
class Participant;
class MessageProcessingInfo;

class MessageReceiver {
public:
  GuidPrefix_t sourceGuidPrefix = GUIDPREFIX_UNKNOWN;
  ProtocolVersion_t sourceVersion = PROTOCOLVERSION;
  VendorId_t sourceVendor = VENDOR_UNKNOWN;
  bool haveTimeStamp = false;

  explicit MessageReceiver(Participant *part);

  bool processMessage(const uint8_t *data, DataSize_t size);

private:
  Participant *mp_part;

  void resetState();

  // TODO make msgInfo a member
  // This probably make processing faster, as no parameter needs to be passed
  // around However, we need to make sure data is set to nullptr after
  // processMsg to make sure we don't access it again afterwards.
  /**
   * Check header for validity, modifies the state of the receiver and
   * adjusts the position of msgInfo accordingly
   */
  bool processHeader(MessageProcessingInfo &msgInfo);
  bool processSubmessage(MessageProcessingInfo &msgInfo,
                         const SubmessageHeader &submsgHeader);
  bool processDataSubmessage(MessageProcessingInfo &msgInfo,
                             const SubmessageHeader &submsgHeader);
  bool processHeartbeatSubmessage(MessageProcessingInfo &msgInfo);
  bool processAckNackSubmessage(MessageProcessingInfo &msgInfo);
};
} // namespace rtps

#endif // RTPS_MESSAGERECEIVER_H

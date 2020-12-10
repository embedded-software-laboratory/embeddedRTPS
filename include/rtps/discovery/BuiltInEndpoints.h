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

#ifndef RTPS_BUILTINENDPOINTS_H
#define RTPS_BUILTINENDPOINTS_H

#include "rtps/entities/StatefulReader.h"
#include "rtps/entities/StatelessReader.h"
#include "rtps/entities/StatelessWriter.h"
#include "rtps/entities/Writer.h"

namespace rtps {

struct BuiltInEndpoints {
  Writer *spdpWriter = nullptr;
  Reader *spdpReader = nullptr;
  Writer *sedpPubWriter = nullptr;
  Reader *sedpPubReader = nullptr;
  Writer *sedpSubWriter = nullptr;
  Reader *sedpSubReader = nullptr;
};
} // namespace rtps

#endif // RTPS_BUILTINENDPOINTS_H

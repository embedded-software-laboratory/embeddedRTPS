/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/entities/Participant.h"
#include "test/mocking/WriterMock.h"
#include "test/mocking/ReaderMock.h"

using rtps::SEDPAgent;
using rtps::Participant;
using testing::_;




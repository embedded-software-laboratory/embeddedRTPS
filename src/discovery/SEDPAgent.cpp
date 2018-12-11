/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/SEDPAgent.h"
#include "rtps/entities/Reader.h"

using rtps::SEDPAgent;

void SEDPAgent::init(rtps::BuiltInEndpoints endpoints) {
    endpoints.sedpSubReader->registerCallback(receiveCallback, this);
    endpoints.sedpPubReader->registerCallback(receiveCallback, this);
}

void SEDPAgent::receiveCallback(void* /*callee*/, ReaderCacheChange& /*cacheChange*/){

}
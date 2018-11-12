/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/SPDP.h"

using rtps::SPDPAgent;

SPDPAgent::SPDPAgent(rtps::ThreadPool &threadPool)
    : writer(TopicKind_t::WITH_KEY, DEFAULT_MULTICAST_LOCATOR, &threadPool){

}

void SPDPAgent::start(){
    if(running){
        return;
    }
    running = true;
    sys_thread_new("SPDPThread", run, this, Config::SPDP_WRITER_STACKSIZE, Config::SPDP_WRITER_PRIO);
}

void SPDPAgent::stop(){
    running = false;
}

void SPDPAgent::run(void* args){
    SPDPAgent& agent = *static_cast<SPDPAgent*>(args);
    agent.writer.newChange(ChangeKind_t::ALIVE, agent.testData.data(), agent.testData.size());

    while(agent.running){
        sys_msleep(Config::SPDP_RESEND_PERIOD_MS);
        agent.writer.unsentChangesReset();
    }
}
/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/discovery/SPDP.h"
#include "rtps/messages/Messages.h"
#include "rtps/utils/udpUtils.h"
#include "rtps/entities/Participant.h"
#include "rtps/entities/Writer.h"
#include "lwip/sys.h"

using rtps::SPDPAgent;
using rtps::SMElement::ParameterId;
using rtps::SMElement::BuildInEndpointSet;

void SPDPAgent::init(Participant& participant) {
    mp_participant = &participant;
    mp_writer = participant.getSPDPWriter();
    ucdr_init_buffer(&m_microbuffer, m_buffer.data(), m_buffer.size());

    //addInlineQos();
    addParticipantParameters();
}

void SPDPAgent::endCurrentList(){
    ucdr_serialize_uint16_t(&m_microbuffer, ParameterId::PID_SENTINEL);
    ucdr_serialize_uint16_t(&m_microbuffer, 0);
}

void SPDPAgent::start(){
    if(m_running){
        return;
    }
    m_running = true;
    sys_thread_new("SPDPThread", run, this, Config::SPDP_WRITER_STACKSIZE, Config::SPDP_WRITER_PRIO);
}

void SPDPAgent::stop(){
    m_running = false;
}

void SPDPAgent::run(void* args){
    SPDPAgent& agent = *static_cast<SPDPAgent*>(args);
    const auto size = ucdr_buffer_length(&agent.m_microbuffer);
    agent.mp_writer->newChange(ChangeKind_t::ALIVE, agent.m_microbuffer.init, size);

    while(agent.m_running){
        sys_msleep(Config::SPDP_RESEND_PERIOD_MS);
        agent.mp_writer->unsentChangesReset();
    }
}


void SPDPAgent::addInlineQos(){
    ucdr_serialize_uint16_t(&m_microbuffer, ParameterId::PID_KEY_HASH);
    ucdr_serialize_uint16_t(&m_microbuffer, 16);
    ucdr_serialize_array_uint8_t(&m_microbuffer, mp_participant->guidPrefix.id.data(), sizeof(GuidPrefix_t::id));
    ucdr_serialize_array_uint8_t(&m_microbuffer, ENTITYID_BUILD_IN_PARTICIPANT.entityKey.data(), sizeof(EntityId_t::entityKey));
    ucdr_serialize_uint8_t(&m_microbuffer,       static_cast<uint8_t>(ENTITYID_BUILD_IN_PARTICIPANT.entityKind));

    endCurrentList();
}

void SPDPAgent::addParticipantParameters(){
    const uint16_t zero_options = 0;
    const uint16_t protocolVersionSize = sizeof(PROTOCOLVERSION.major) + sizeof(PROTOCOLVERSION.minor);
    const uint16_t vendorIdSize = Config::VENDOR_ID.vendorId.size();
    const uint16_t locatorSize = sizeof(Locator_t);
    const uint16_t durationSize = sizeof(Behavior::Duration_t::seconds) + sizeof(Behavior::Duration_t::fraction);
    const uint16_t guidSize = sizeof(GuidPrefix_t::id) + sizeof(EntityId_t::entityKey) + sizeof(EntityId_t::entityKind);

    const Locator_t userUniCastLocator = getUserUnicastLocator(mp_participant->participantId);
    const Locator_t builtInUniCastLocator = getBuiltInUnicastLocator(mp_participant->participantId);
    const Locator_t builtInMultiCastLocator = getBuiltInMulticastLocator();

    ucdr_serialize_array_uint8_t(&m_microbuffer, rtps::SMElement::SCHEME_PL_CDR_LE.data(), rtps::SMElement::SCHEME_PL_CDR_LE.size());
    ucdr_serialize_uint16_t(&m_microbuffer, zero_options);

    ucdr_serialize_uint16_t(&m_microbuffer, ParameterId::PID_PROTOCOL_VERSION);
    ucdr_serialize_uint16_t(&m_microbuffer, protocolVersionSize);
    ucdr_serialize_uint8_t(&m_microbuffer,  PROTOCOLVERSION.major);
    ucdr_serialize_uint8_t(&m_microbuffer,  PROTOCOLVERSION.minor);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_VENDORID);
    ucdr_serialize_uint16_t(&m_microbuffer,      vendorIdSize);
    ucdr_serialize_array_uint8_t(&m_microbuffer, Config::VENDOR_ID.vendorId.data(), vendorIdSize);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_DEFAULT_UNICAST_LOCATOR);
    ucdr_serialize_uint16_t(&m_microbuffer,      locatorSize);
    ucdr_serialize_array_uint8_t(&m_microbuffer, reinterpret_cast<const uint8_t*>(&userUniCastLocator), locatorSize);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_METATRAFFIC_UNICAST_LOCATOR);
    ucdr_serialize_uint16_t(&m_microbuffer,      locatorSize);
    ucdr_serialize_array_uint8_t(&m_microbuffer, reinterpret_cast<const uint8_t*>(&builtInUniCastLocator), locatorSize);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_METATRAFFIC_MULTICAST_LOCATOR);
    ucdr_serialize_uint16_t(&m_microbuffer,      locatorSize);
    ucdr_serialize_array_uint8_t(&m_microbuffer, reinterpret_cast<const uint8_t*>(&builtInMultiCastLocator), locatorSize);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_PARTICIPANT_LEASE_DURATION);
    ucdr_serialize_uint16_t(&m_microbuffer,      durationSize);
    ucdr_serialize_int32_t(&m_microbuffer,       Config::SPDP_LEASE_DURATION.seconds);
    ucdr_serialize_uint32_t(&m_microbuffer,      Config::SPDP_LEASE_DURATION.fraction);

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_PARTICIPANT_GUID);
    ucdr_serialize_uint16_t(&m_microbuffer,      guidSize);
    ucdr_serialize_array_uint8_t(&m_microbuffer, mp_participant->guidPrefix.id.data(), sizeof(GuidPrefix_t::id));
    ucdr_serialize_array_uint8_t(&m_microbuffer, ENTITYID_BUILD_IN_PARTICIPANT.entityKey.data(), sizeof(EntityId_t::entityKey));
    ucdr_serialize_uint8_t(&m_microbuffer,       static_cast<uint8_t>(ENTITYID_BUILD_IN_PARTICIPANT.entityKind));

    ucdr_serialize_uint16_t(&m_microbuffer,      ParameterId::PID_BUILTIN_ENDPOINT_SET);
    ucdr_serialize_uint16_t(&m_microbuffer,      sizeof(BuildInEndpointSet));
    ucdr_serialize_uint32_t(&m_microbuffer,      BuildInEndpointSet::DISC_BIE_PARTICIPANT_ANNOUNCER |
                                               BuildInEndpointSet::DISC_BIE_PARTICIPANT_DETECTOR |
                                               BuildInEndpointSet::DISC_BIE_PUBLICATION_ANNOUNCER |
                                               BuildInEndpointSet::DISC_BIE_PUBLICATION_DETECTOR |
                                               BuildInEndpointSet::DISC_BIE_SUBSCRIPTION_ANNOUNCER |
                                               BuildInEndpointSet::DISC_BIE_SUBSCRIPTION_DETECTOR);

    endCurrentList();
}



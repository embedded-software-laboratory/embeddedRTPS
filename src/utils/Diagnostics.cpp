#include <rtps/utils/Diagnostics.h>

namespace rtps {
namespace Diagnostics {

namespace ThreadPool {
uint32_t dropped_packets_usertraffic;
uint32_t dropped_packets_metatraffic;

uint32_t max_ever_elements_outgoing_usertraffic_queue;
uint32_t max_ever_elements_incoming_usertraffic_queue;

uint32_t max_ever_elements_outgoing_metatraffic_queue;
uint32_t max_ever_elements_incoming_metatraffic_queue;

} // namespace ThreadPool

namespace StatefulReader {
uint32_t sfr_unexpected_sn;
uint32_t sfr_retransmit_requests;
} // namespace StatefulReader

namespace Network {
uint32_t lwip_allocation_failures;
}

namespace SEDP {
uint32_t max_ever_remote_participants;
uint32_t current_remote_participants;

uint32_t max_ever_matched_reader_proxies;
uint32_t current_max_matched_reader_proxies;

uint32_t max_ever_matched_writer_proxies;
uint32_t current_max_matched_writer_proxies;

uint32_t max_ever_unmatched_reader_proxies;
uint32_t current_max_unmatched_reader_proxies;

uint32_t max_ever_unmatched_writer_proxies;
uint32_t current_max_unmatched_writer_proxies;
} // namespace SEDP

} // namespace Diagnostics
} // namespace rtps

/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/rtps.h"
#include "rtps/UdpDriver.h"
#include "default_netif.h"
#include "lwip/sys.h"


void callback(void *arg, udp_pcb *pcb, pbuf *p,
              const ip_addr_t *addr, u16_t port){
    printf("Received something from %s:%u !!!!\n\r", ipaddr_ntoa(addr), port);
    for(int i=0; i < p->len; i++){
        printf("%i ", ((uint8_t*)p->payload)[i]);
    }
    pbuf_free(p);
}

int main(){
    rtps::init();

    UdpDriver udpDriver;
    const uint16_t port = 7050;
    ip4_addr addr;
    IP4_ADDR((&addr), 192,168,0,248);
    udpDriver.createUdpConnection(addr, port, callback);

    const std::array<uint8_t, 8> data = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    while (!LWIP_EXAMPLE_APP_ABORT()) {
        udpDriver.sendPacket(addr, port, data._M_elems, data.size());
        default_netif_poll();
    }
    default_netif_shutdown();
}

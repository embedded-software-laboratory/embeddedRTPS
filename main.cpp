/*
 *
 * Author: Andreas Wuestenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include <lwip/tcpip.h>
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "lwipcfg.h"
#include "default_netif.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/sys.h"
#include "arch/sys_arch.h"

#include "rtps/UdpDriver.h"

#include <time.h>

static void test_netif_init(){
    ip4_addr_t ipaddr, netmask, gw;
    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

    LWIP_PORT_INIT_GW(&gw);
    LWIP_PORT_INIT_IPADDR(&ipaddr);
    LWIP_PORT_INIT_NETMASK(&netmask);
    printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));
    init_default_netif(&ipaddr, &netmask, &gw);

    netif_set_up(netif_default);
}

static void test_init(void * arg){
    sys_sem_t *init_sem;
    LWIP_ASSERT("arg != NULL", arg != NULL);
    init_sem = (sys_sem_t*)arg;

    srand((unsigned int)time(0));
    test_netif_init();
    sys_sem_signal(init_sem);
}

void start(){
    /* no stdio-buffering, please! */
    setvbuf(stdout, NULL,_IONBF, 0);

    err_t err;
    sys_sem_t init_sem;

    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
    LWIP_UNUSED_ARG(err);
    tcpip_init(test_init, &init_sem);
    /* we have to wait for initialization to finish before
   * calling update_adapter()! */
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
}

int main(){
    start();

    UdpDriver udpDriver;
    const uint16_t port = 7050;
    ip4_addr addr;
    IP4_ADDR((&addr), 192,168,0,248);
    udpDriver.createUdpConnection(addr, port);

    const std::array<uint8_t, 8> data = {'d', 'e', 'a', 'd', 'b', 'e', 'e', 'f'};
    while (!LWIP_EXAMPLE_APP_ABORT()) {
        udpDriver.sendPacket(addr, port, data);
        default_netif_poll();
    }
    default_netif_shutdown();
}

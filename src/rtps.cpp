/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#include "rtps/rtps.h"
#include "lwip/ip4_addr.h"
#include "lwip/sys.h"
#include "lwipcfg.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include <time.h>
#include <lwip/tcpip.h>

#ifdef HIGHTEC_TOOLCHAIN
    #include "ethernetif.h"
#else
    #include "default_netif.h"
    #include "../pcapif.h"
#endif

static struct netif netif;

static void init(void* arg){
    if(arg == nullptr){
        printf("Failed to init. nullptr passed");
    }
    sys_sem_t *init_sem = static_cast<sys_sem_t*>(arg);

    srand((unsigned int)time(0));

    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;
    LWIP_PORT_INIT_GW(&gw);
    LWIP_PORT_INIT_IPADDR(&ipaddr);
    LWIP_PORT_INIT_NETMASK(&netmask);
    printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));

#ifdef HIGHTEC_TOOLCHAIN
    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
#else
    netif_add(&netif, &ipaddr, &netmask, &gw, NULL, pcapif_init, tcpip_input);
#endif
    netif_set_default(&netif);
    netif_set_up(netif_default);

    sys_sem_signal(init_sem);
}

void LwIPInit(){
    /* no stdio-buffering, please! */
    setvbuf(stdout, NULL,_IONBF, 0);

    err_t err;
    sys_sem_t init_sem;

    err = sys_sem_new(&init_sem, 0);
    LWIP_ASSERT("failed to create init_sem", err == ERR_OK);
    LWIP_UNUSED_ARG(err);
    tcpip_init(init, &init_sem);
    /* we have to wait for initialization to finish before
   * calling update_adapter()! */
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
}

void rtps::init(){
    LwIPInit();
}

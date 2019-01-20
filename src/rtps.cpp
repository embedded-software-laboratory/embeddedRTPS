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
#include <cmath>

#ifdef HIGHTEC_TOOLCHAIN
    #include "ethernetif.h"
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #include "default_netif.h"
    #include "../pcapif.h"
#else
    #include "netif/tapif.h"
#endif

#define INIT_VERBOSE 0

static struct netif netif;

static void init(void* arg){
    if(arg == nullptr){
#if INIT_VERBOSE
        printf("Failed to init. nullptr passed");
#endif
        return;
    }
    auto init_sem = static_cast<sys_sem_t*>(arg);

    srand((unsigned int)time(nullptr));

    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gw;
    LWIP_PORT_INIT_GW(&gw);
    LWIP_PORT_INIT_IPADDR(&ipaddr);
    LWIP_PORT_INIT_NETMASK(&netmask);
#if INIT_VERBOSE
    printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));
#endif

#ifdef HIGHTEC_TOOLCHAIN
    netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, ethernetif_init, tcpip_input);
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, pcapif_init, tcpip_input);
#else
    netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, tapif_init, tcpip_input);
#endif

    netif_set_default(&netif);
    netif_set_up(netif_default);

    sys_sem_signal(init_sem);
}

void LwIPInit(){
    /* no stdio-buffering, please! */
    setvbuf(stdout, nullptr ,_IONBF, 0);

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
    // TODO This is not threadsafe. Might cause problems in tests. For now, it seems to work.
    static bool initialized = false;
    if(!initialized){
        LwIPInit();
        initialized = true;
    }
}

rtps::Time_t rtps::getCurrentTimeStamp(){
    Time_t now;
    // TODO FIX
    uint32_t nowMs = sys_now();
    now.seconds = (int32_t) nowMs / 1000;
    now.fraction = ((nowMs % 1000)/1000);
    return now;
}


#undef INIT_VERBOSE
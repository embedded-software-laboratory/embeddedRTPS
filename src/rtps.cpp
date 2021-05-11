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

#include "rtps/rtps.h"
#include "lwip/sys.h"
#include <cmath>
#include <time.h>

// Currently initialization is expected to be done in project main for e.g.
// Aurix and STM32
#if defined(unix) || defined(__unix__) || defined(WIN32) || defined(_WIN32) || \
    defined(__WIN32) && !defined(__CYGWIN__)

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwipcfg.h"
#include <lwip/tcpip.h>

#if defined(unix) || defined(__unix__)
#include "netif/tapif.h"
#elif defined(WIN32) || defined(_WIN32) ||                                     \
    defined(__WIN32) && !defined(__CYGWIN__)
#include "../pcapif.h"
#include "default_netif.h"
#else
#include "ethernetif.h"
#endif

#define INIT_VERBOSE 0

static struct netif netif;

static void init(void *arg) {
  if (arg == nullptr) {
#if INIT_VERBOSE
    printf("Failed to init. nullptr passed");
#endif
    return;
  }
  auto init_sem = static_cast<sys_sem_t *>(arg);

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

#if defined(unix) || defined(__unix__)
  netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, tapif_init, tcpip_input);
#elif defined(WIN32) || defined(_WIN32) ||                                     \
    defined(__WIN32) && !defined(__CYGWIN__)
  netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, pcapif_init, tcpip_input);
#else
  netif_add(&netif, &ipaddr, &netmask, &gw, nullptr, ethernetif_init,
            tcpip_input);
#endif

  netif_set_default(&netif);
  netif_set_up(netif_default);

  sys_sem_signal(init_sem);
}

void LwIPInit() {
  /* no stdio-buffering, please! */
  setvbuf(stdout, nullptr, _IONBF, 0);

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

void rtps::init() {
  // TODO This is not threadsafe. Might cause problems in tests. For now, it
  // seems to work.
  static bool initialized = false;
  if (!initialized) {
    LwIPInit();
    initialized = true;
  }
}
#endif

#undef INIT_VERBOSE

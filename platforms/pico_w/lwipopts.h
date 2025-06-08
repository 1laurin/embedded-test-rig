#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Platform specific configuration for Pico W
#include "lwipopts_examples_common.h"

// Memory management - CRITICAL settings
#define MEM_LIBC_MALLOC 0
#define MEMP_MEM_MALLOC 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE 4000

// TCP/IP settings
#define MEMP_NUM_TCP_PCB 10
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_UDP_PCB 4
#define PBUF_POOL_SIZE 24
#define TCP_MSS 1460
#define TCP_WND (4 * TCP_MSS)
#define TCP_SND_BUF (4 * TCP_MSS)

// Enable necessary protocols
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DHCP 1
#define LWIP_DNS 1

// WebSocket support
#define LWIP_HTTPD 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI 1

#endif /* _LWIPOPTS_H */

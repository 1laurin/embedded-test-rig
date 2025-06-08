/**
 * @file lwipopts.h
 * @brief lwIP configuration options for Raspberry Pi Pico W
 *
 * This file configures the lwIP TCP/IP stack for the Pico W WiFi functionality.
 * Place this file in platforms/pico_w/include/ directory.
 */

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// Platform and architecture settings
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_NETIF_API 0

// Memory settings
#define MEM_LIBC_MALLOC 1
#define MEMP_MEM_MALLOC 1
#define MEM_ALIGNMENT 4
#define MEM_SIZE 4000
#define MEMP_NUM_TCP_SEG 32
#define MEMP_NUM_ARP_QUEUE 10
#define PBUF_POOL_SIZE 24
#define LWIP_ARP 1
#define LWIP_ETHERNET 1
#define LWIP_ICMP 1
#define LWIP_RAW 1
#define TCP_WND (8 * TCP_MSS)
#define TCP_MSS 1460
#define TCP_SND_BUF (8 * TCP_MSS)
#define TCP_SND_QUEUELEN ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DNS 1
#define LWIP_TCP_KEEPALIVE 1

// DHCP settings
#define LWIP_DHCP 1
#define DHCP_DOES_ARP_CHECK 0

// IPv4 settings
#define LWIP_IPV4 1
#define LWIP_IPV6 0

// HTTP server settings for WebSocket support
#define LWIP_HTTPD 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI 1
#define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#define LWIP_HTTPD_SUPPORT_WEBSOCKET 1

// TCP settings for reliable connections
#define LWIP_TCP_TIMESTAMPS 0
#define TCP_QUEUE_OOSEQ 0
#define LWIP_TCP_SACK_OUT 0

// UDP settings
#define LWIP_UDP 1
#define LWIP_UDPLITE 0

// Statistics and debugging
#define LWIP_STATS 0
#define LWIP_STATS_DISPLAY 0

// Threading (we're using NO_SYS=1, so these are disabled)
#define LWIP_TCPIP_CORE_LOCKING 0
#define LWIP_TCPIP_CORE_LOCKING_INPUT 0

// Checksum settings (let hardware handle checksums when possible)
#define CHECKSUM_GEN_IP 0
#define CHECKSUM_GEN_UDP 0
#define CHECKSUM_GEN_TCP 0
#define CHECKSUM_GEN_ICMP 0
#define CHECKSUM_CHECK_IP 0
#define CHECKSUM_CHECK_UDP 0
#define CHECKSUM_CHECK_TCP 0
#define CHECKSUM_CHECK_ICMP 0

// Pico W specific settings
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1

// Buffer settings optimized for Pico W
#define PBUF_LINK_HLEN 14
#define PBUF_POOL_BUFSIZE LWIP_MEM_ALIGN_SIZE(TCP_MSS + 40 + PBUF_LINK_HLEN)

// Timeout settings
#define MEMP_NUM_SYS_TIMEOUT (LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2 * LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS + (PPP_SUPPORT * 6 * MEMP_NUM_PPP_PCB) + (LWIP_IPV6 ? (1 + LWIP_IPV6_REASS + LWIP_IPV6_MLD) : 0))

// Application specific settings for diagnostic rig
#define LWIP_CALLBACK_API 1
#define LWIP_EVENT_API 0

// Multicast settings (useful for discovery protocols)
#define LWIP_IGMP 1

// Error handling
#define LWIP_PROVIDE_ERRNO 1

// Memory debugging (disable for production)
#define MEMP_OVERFLOW_CHECK 0
#define MEMP_SANITY_CHECK 0

// Custom settings for our WebSocket server
#define LWIP_HTTPD_MAX_TAG_NAME_LEN 16
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN 192
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 512

// Network interface settings
#define LWIP_SINGLE_NETIF 1
#define LWIP_BROADCAST_PING 1
#define LWIP_MULTICAST_PING 1

// DNS settings
#define DNS_TABLE_SIZE 4
#define DNS_MAX_NAME_LENGTH 256

// SNMP (disabled for our application)
#define LWIP_SNMP 0

// PPP (disabled for WiFi)
#define PPP_SUPPORT 0

// Network time protocol (disabled)
#define LWIP_SNTP 0

// IPv6 (disabled to save memory)
#define LWIP_IPV6 0
#define LWIP_IPV6_DHCP6 0

// Application layered TCP (disabled)
#define LWIP_ALTCP 0

// Custom allocator settings
#define LWIP_ALLOW_MEM_FREE_FROM_OTHER_CONTEXT 1

// Performance optimizations for Pico W
#define LWIP_WND_SCALE 0
#define TCP_RCV_SCALE 0
#define LWIP_TCP_RCV_SCALE 0

// Debugging options (set to 1 to enable debug output)
#define LWIP_DEBUG 0
#define LWIP_DBG_MIN_LEVEL LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_TYPES_ON LWIP_DBG_ON

#if LWIP_DEBUG
#define TCP_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF
#define ICMP_DEBUG LWIP_DBG_OFF
#define DHCP_DEBUG LWIP_DBG_OFF
#define DNS_DEBUG LWIP_DBG_OFF
#define HTTPD_DEBUG LWIP_DBG_ON
#endif

#endif /* LWIPOPTS_H */
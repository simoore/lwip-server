#pragma once

#define LWIP_TESTMODE                   1

#define LWIP_IPV6                       0

#define LWIP_CHECKSUM_ON_COPY           1
#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK 1
#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL(printfmsg) LWIP_ASSERT("TCP_CHECKSUM_ON_COPY_SANITY_CHECK_FAIL", 0)

/* We link to special sys_arch.c (for basic non-waiting API layers unit tests) */
#define NO_SYS                          1
#define SYS_LIGHTWEIGHT_PROT            0
#define LWIP_NETCONN                    !NO_SYS
#define LWIP_SOCKET                     !NO_SYS
#define LWIP_NETCONN_FULLDUPLEX         LWIP_SOCKET
#define LWIP_NETBUF_RECVINFO            1
#define LWIP_HAVE_LOOPIF                1
#define TCPIP_THREAD_TEST

/* Enable DHCP to test it, disable UDP checksum to easier inject packets */
#define LWIP_DHCP                       1

/* Minimal changes to opt.h required for tcp unit tests: */
#define MEM_SIZE                        16000
#define TCP_SND_QUEUELEN                40
#define MEMP_NUM_TCP_SEG                TCP_SND_QUEUELEN
#define TCP_SND_BUF                     (12 * TCP_MSS)
#define TCP_WND                         (10 * TCP_MSS)
#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   0
#define PBUF_POOL_SIZE                  400 /* pbuf tests need ~200KByte */

/* Enable IGMP and MDNS for MDNS tests */
#define LWIP_IGMP                       1
#define LWIP_MDNS_RESPONDER             1
#define LWIP_NUM_NETIF_CLIENT_DATA      (LWIP_MDNS_RESPONDER)

/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES   1

#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 8)

/* MIB2 stats are required to check IPv4 reassembly results */
#define MIB2_STATS                      1

/* netif tests want to test this, so enable: */
#define LWIP_NETIF_EXT_STATUS_CALLBACK  1

/* Check lwip_stats.mem.illegal instead of asserting */
#define LWIP_MEM_ILLEGAL_FREE(msg)      /* to nothing */

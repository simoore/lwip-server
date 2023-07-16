#pragma once

// Do not use LwIP profiling functions.
#define LWIP_PERF 0

// Debugging options.
#define LWIP_DEBUG
#define ETHARP_DEBUG LWIP_DBG_ON
#define TIMERS_DEBUG LWIP_DBG_ON

// LWIP_STATS turns on functionality to collect statistics from modules within LwIP for debugging and profiling.
#define LWIP_STATS 0

// Socket is one of LwIP APIs. It is not available for bare-metal applications.
#define LWIP_SOCKET 0

// Enable LwIP modules.
#define LWIP_ICMP 1
#define LWIP_DHCP 1
#define LWIP_IPV4 1
#define LWIP_ARP 1
#define LWIP_ETHERNET 1

// SYS_LIGHTWEIGHT_PROT refers to inter-task protection for certain critical memory regions during memory allocation & 
// deallocation.
#define SYS_LIGHTWEIGHT_PROT 0

/*****************************************************************************/
/********** MEMORY OPTIONS ***************************************************/
/*****************************************************************************/

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (10*1024)

/* Relocate the LwIP RAM heap pointer */
#define LWIP_RAM_HEAP_POINTER    (0x30044000)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           10
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        6
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 6
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        8
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    10

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool.
   @ note: used to allocate Tx pbufs only */
#define PBUF_POOL_SIZE          8

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool */
#define PBUF_POOL_BUFSIZE       1528

// Support custom PBUF's to pass directly MAC Rx buffers to the stack no copy is needed.
#define LWIP_SUPPORT_CUSTOM_PBUF 1

#define LWIP_NETIF_LINK_CALLBACK        1

/*****************************************************************************/
/********** TCP & UDP OPTIONS ************************************************/
/*****************************************************************************/

#define LWIP_UDP                1
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (4*TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */
#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)

/* ---------- UDP options ---------- */
#define UDP_TTL                 255

/*****************************************************************************/
/********** CHECKSUM OPTIONS *************************************************/
/*****************************************************************************/

/* 
The STM32H7xx allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE 
/* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
#define CHECKSUM_GEN_IP                 0
/* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
#define CHECKSUM_GEN_UDP                0
/* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
#define CHECKSUM_GEN_TCP                0 
/* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
#define CHECKSUM_CHECK_IP               0
/* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
#define CHECKSUM_CHECK_UDP              0
/* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
#define CHECKSUM_CHECK_TCP              0
/* CHECKSUM_GEN_ICMP==1: Check checksums by hardware for outgoing ICMP packets.*/
/* Hardware TCP/UDP checksum insertion not supported when packet is an IPv4 fragment*/
#define CHECKSUM_GEN_ICMP               1
/* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/
#define CHECKSUM_CHECK_ICMP             0

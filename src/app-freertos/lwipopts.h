#pragma once

#include "lwipopts_common.h"
#include "FreeRTOS.h"

// NO_SYS provides minimal functionality. This is the suitable option of bare-metal applications. NO_SYS removes 
// functionality like using the system timers, threads, mutexes, and memory management etc.
#define NO_SYS 0

// Netconn is one of LwIP APIs. It is not available for bare-metal applications.
#define LWIP_NETCONN 1

// Use our own timer to generate sys_now() for LwIP
#define LWIP_FREERTOS_SYS_NOW_FROM_FREERTOS 0

/*****************************************************************************/
/********** FREERTOS OPTIONS *************************************************/
/*****************************************************************************/

#define TCPIP_THREAD_NAME              "TCP/IP"
#define TCPIP_THREAD_STACKSIZE          2048
#define TCPIP_MBOX_SIZE                 6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       6
#define DEFAULT_ACCEPTMBOX_SIZE         6
#define DEFAULT_THREAD_STACKSIZE        1024
#define TCPIP_THREAD_PRIO               PRIORITY_HIGH
#pragma once

#include "lwipopts_common.h"

// NO_SYS provides minimal functionality. This is the suitable option of bare-metal applications. NO_SYS removes 
// functionality like using the system timers, threads, mutexes, and memory management etc.
#define NO_SYS 1

// Netconn is one of LwIP APIs. It is not available for bare-metal applications.
#define LWIP_NETCONN 0

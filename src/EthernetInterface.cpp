#include "EthernetInterface.h"

/// Allocation of RX buffers in LwIP heap.
LWIP_MEMPOOL_DECLARE(sRxPool, EthernetInterface::sRxBufferCount, sizeof(EthernetInterface::RxBuf), "Zero-copy RX PBUF pool");


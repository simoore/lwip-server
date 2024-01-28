#include <cstdint>
#include <cstring>
#include <span>

#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "stm32h7xx_hal.h"

#include "lwipserver/drivers/Lan8742.h"
#include "lwipserver/stm32h7/Base.h"

/*****************************************************************************/
/********** CONSTANTS ********************************************************/
/*****************************************************************************/

static constexpr uint32_t sEthTxDescCount = 24;

class TxDescriptor {
public:

    static constexpr uint32_t sOWNMask = 0x80000000;

    void set(std::span<uint8_t> buf1, std::span<uint8_t> buf2) {
        mDesc0 = reinterpret_cast<uint32_t>(buf1.data());
        mDesc1 = reinterpret_cast<uint32_t>(buf2.data());
    }

    bool ownedByDMA(void) const {
        return (mDesc3 & sOWNMask) == sOWNMask;
    }

private:

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    volatile uint32_t mDesc0;
    volatile uint32_t mDesc1;
    volatile uint32_t mDesc2;
    volatile uint32_t mDesc3;
    uint32_t mAppData0;
    uint32_t mAppData1;

} __attribute__((packed));

static_assert(sizeof(TxDescriptor) == 24);

#define IFNAME0 's'
#define IFNAME1 't'

#define ETH_DMA_TRANSMIT_TIMEOUT (20U)

#define ETH_RX_BUFFER_SIZE 1000U
#define ETH_RX_BUFFER_CNT 12U
#define ETH_TX_BUFFER_MAX ((ETH_TX_DESC_CNT)*2U)

/*
@Note: This interface is implemented to operate in zero-copy mode only:
        - Rx Buffers will be allocated from LwIP stack memory heap, then passed to ETH HAL driver.
        - Tx Buffers will be allocated from LwIP stack memory heap, then passed to ETH HAL driver.

@Notes:
  1.a. ETH DMA Rx descriptors must be contiguous, the default count is 4,
       to customize it please redefine ETH_RX_DESC_CNT in ETH GUI (Rx Descriptor Length)
       so that updated value will be generated in stm32xxxx_hal_conf.h
  1.b. ETH DMA Tx descriptors must be contiguous, the default count is 4,
       to customize it please redefine ETH_TX_DESC_CNT in ETH GUI (Tx Descriptor Length)
       so that updated value will be generated in stm32xxxx_hal_conf.h

  2.a. Rx Buffers number: ETH_RX_BUFFER_CNT must be greater than ETH_RX_DESC_CNT.
  2.b. Rx Buffers must have the same size: ETH_RX_BUFFER_SIZE, this value must
       passed to ETH DMA in the init field (heth.Init.RxBuffLen)
*/

typedef enum { RX_ALLOC_OK = 0x00, RX_ALLOC_ERROR = 0x01 } RxAllocStatusTypeDef;

typedef struct {
    struct pbuf_custom pbuf_custom;
    uint8_t buff[(ETH_RX_BUFFER_SIZE + 31) & ~31] __ALIGNED(32);
} RxBuff_t;

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); 
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));

//TxDescriptor mTxDescriptors[sEthTxDescCount] __attribute__((section(".TxDecripSection")));

// Memory Pool Declaration
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");
__attribute__((section(".Rx_PoolSection"))) extern u8_t memp_memory_RX_POOL_base[];

/* Variable Definitions */
static uint8_t RxAllocStatus;

/* Global Ethernet handle*/
ETH_HandleTypeDef EthHandle;
ETH_TxPacketConfig TxConfig;

class Ether {
public: 

    /// Read a register from the ethernet phy.
    /// 
    /// @param devAddr
    ///		The device address.
    /// @param regAddr
    ///		The register address.
    /// @return
    ///		If the read was successful and the register value if the read succeeded.
    static std::pair<bool, uint32_t> readReg(uint32_t devAddr, uint32_t regAddr) {
        uint32_t regval;
        if (HAL_ETH_ReadPHYRegister(&EthHandle, devAddr, regAddr, &regval) != HAL_OK) {
            return std::make_pair(false, 0);
        }
        return std::make_pair(true, regval);
    }

    /// Write to a register on the ethernet phy.
    ///
    /// @param devAddr
    ///		The device address.
    /// @param regAddr
    ///		The register address.
    /// @param regVal
    ///		The value to write to the register.
    /// @return
    ///		If the write was successful.
    static bool writeReg(uint32_t devAddr, uint32_t regAddr, uint32_t regVal) {
        if (HAL_ETH_WritePHYRegister(&EthHandle, devAddr, regAddr, regVal) != HAL_OK) {
            return false;
        }
        return true;
    }
};


lwipserver::drivers::Lan8742 sLan8742;

/* Private functions ---------------------------------------------------------*/
void pbuf_free_custom(struct pbuf *p);
void ethernet_link_check_state(struct netif *netif);

/*******************************************************************************
                       LL Driver Interface ( LwIP stack --> ETH)
*******************************************************************************/
/**
 * @brief In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif) {
    uint8_t macaddress[6] = {ETH_MAC_ADDR0, ETH_MAC_ADDR1, ETH_MAC_ADDR2, ETH_MAC_ADDR3, ETH_MAC_ADDR4, ETH_MAC_ADDR5};

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = macaddress;
    EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
    EthHandle.Init.RxDesc = DMARxDscrTab;
    EthHandle.Init.TxDesc = DMATxDscrTab;
    EthHandle.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    HAL_ETH_Init(&EthHandle);

    /* set MAC hardware address length */
    netif->hwaddr_len = ETH_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = ETH_MAC_ADDR0;
    netif->hwaddr[1] = ETH_MAC_ADDR1;
    netif->hwaddr[2] = ETH_MAC_ADDR2;
    netif->hwaddr[3] = ETH_MAC_ADDR3;
    netif->hwaddr[4] = ETH_MAC_ADDR4;
    netif->hwaddr[5] = ETH_MAC_ADDR5;

    /* maximum transfer unit */
    netif->mtu = ETH_MAX_PAYLOAD;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    /* Initialize the RX POOL */
    LWIP_MEMPOOL_INIT(RX_POOL);

    /* Set Tx packet config common parameters */
    memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
    TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    HAL_ETH_SetMDIOClockRange(&EthHandle);
    sLan8742.init<lwipserver::stm32h7::Base, Ether>();

    ethernet_link_check_state(netif);
}

/// This function transmits the packet. The packet is contained in the pbuf that is passed to the function. This pbuf
/// might be chained.
///
/// @param netif 
///     The lwip network interface structure for this ethernetif.
/// @param p 
///     The MAC packet to send (e.g. IP packet including MAC addresses and type)
/// @return 
///     ERR_OK if the packet could be sent an err_t value if the packet couldn't be sent
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    uint32_t i = 0U;
    struct pbuf *q = nullptr;
    err_t errval = ERR_OK;
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT] = {0};

    memset(Txbuffer, 0, ETH_TX_DESC_CNT * sizeof(ETH_BufferTypeDef));

    for (q = p; q != nullptr; q = q->next) {
        if (i >= ETH_TX_DESC_CNT)
            return ERR_IF;

        Txbuffer[i].buffer = reinterpret_cast<uint8_t *>(q->payload);
        Txbuffer[i].len = q->len;

        if (i > 0) {
            Txbuffer[i - 1].next = &Txbuffer[i];
        }

        if (q->next == NULL) {
            Txbuffer[i].next = nullptr;
        }

        i++;
    }

    TxConfig.Length = p->tot_len;
    TxConfig.TxBuffer = Txbuffer;
    TxConfig.pData = p;

    HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);

    return errval;
}


// /// @brief 
// /// @param netif 
// /// @param p 
// /// @return 
// static err_t lowLevelOutput(struct netif *netif, struct pbuf *p) {
    
//     using PacketBuf = struct pbuf;

//     // When this function returns, LwIP is going to free the buffer. Incrementing the reference count.
//     // TODO: double check that LwIP actually does call pbuf_free when this function returns.
//     uint32_t appDesc = 
//     for (PacketBuf *q = p; q != nullptr; q = q->next) {
//         if (ownedByDMA) {
//             return ERR_IF;
//         }
//         if (i >= ETH_TX_DESC_CNT)
//             return ERR_IF;
//         pbuf_ref(p);
//     }

//     HAL_ETH_Transmit_IT(&EthHandle, &TxConfig);

//     while (semWait(TxPktSemaphore, TIME_WAITING_FOR_INPUT) == false) {}

//     HAL_ETH_ReleaseTxPacket(&EthHandle);
// }


/**
 * @brief Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif) {
    struct pbuf *p = nullptr;

    if (RxAllocStatus == RX_ALLOC_OK) {
        HAL_ETH_ReadData(&EthHandle, (void **)&p);
    }
    return p;
}

/// ethernetif_input is called periodically to read from the network interface and pass packets to the TCP/IP stack.
/// It uses the function low_level_input() that handles the actual reception of bytes from the network interface. 
/// Then the type of the received packet is determined and the appropriate input function is called.
///
/// @param netif 
///     The lwip network interface structure for this ethernetif
void ethernetif_input(struct netif *netif) {
    while (true) {
        struct pbuf *p = low_level_input(netif);
        if (p == nullptr) {
            break;
        }
        if (netif->input(p, netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
}

/**
 * @brief Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

/**
 * @brief  Custom Rx pbuf free callback
 * @param  pbuf: pbuf to be freed
 * @retval None
 */
void pbuf_free_custom(struct pbuf *p) {
    struct pbuf_custom *custom_pbuf = (struct pbuf_custom *)p;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);
    /* If the Rx Buffer Pool was exhausted, signal the ethernetif_input task to
     * call HAL_ETH_GetRxDataBuffer to rebuild the Rx descriptors. */
    if (RxAllocStatus == RX_ALLOC_ERROR) {
        RxAllocStatus = RX_ALLOC_OK;
    }
}

/// This is required by LwIP to get a millisecond tick.
extern "C" u32_t sys_now(void) {
    return HAL_GetTick();
}

/*******************************************************************************
                       Ethernet MSP Routines
*******************************************************************************/
/**
 * @brief  Initializes the ETH MSP.
 * @param  heth: ETH handle
 * @retval None
 */
extern "C" void HAL_ETH_MspInit(ETH_HandleTypeDef *heth) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // Ethernett MSP init: RMII Mode
    // RMII_REF_CLK ----------------------> PA1
    // RMII_MDIO -------------------------> PA2
    // RMII_MDC --------------------------> PC1
    // RMII_MII_CRS_DV -------------------> PA7
    // RMII_MII_RXD0 ---------------------> PC4
    // RMII_MII_RXD1 ---------------------> PC5
    // RMII_MII_TX_EN --------------------> PG11
    // RMII_MII_TXD0 ---------------------> PG13
    // RMII_MII_TXD1 ---------------------> PB13

    /* Enable GPIOs clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* Configure PA1, PA2, PA7 */
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure PB13 */
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure PC1, PC4 and PC5 */
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PG11, and PG13 */
    GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Enable Ethernet clocks */
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();
}

/*******************************************************************************
                       PHI IO Functions
*******************************************************************************/

/**
 * @brief
 * @retval None
 */
void ethernet_link_check_state(struct netif *netif) {
    using Status = lwipserver::drivers::Lan8742::Status;

    ETH_MACConfigTypeDef MACConf = {0};
    uint32_t linkchanged = 0U, speed = 0U, duplex = 0U;

    Status status = sLan8742.getLinkState<Ether>();
    bool linkStatusDown = status == Status::LinkDown || status == Status::ReadError;

    if (netif_is_link_up(netif) && linkStatusDown) {
        HAL_ETH_Stop(&EthHandle);
        netif_set_down(netif);
        netif_set_link_down(netif);
    } else if (!netif_is_link_up(netif) && !linkStatusDown) {
        switch (status) {
        case Status::FullDuplex100Mbit: 
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            linkchanged = 1;
            break;
        case Status::HalfDuplex100Mbit:
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_100M;
            linkchanged = 1;
            break;
        case Status::FullDuplex10Mbit:
            duplex = ETH_FULLDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            linkchanged = 1;
            break;
        case Status::HalfDuplex10Mbit:
            duplex = ETH_HALFDUPLEX_MODE;
            speed = ETH_SPEED_10M;
            linkchanged = 1;
            break;
        default:
            break;
        }

        if (linkchanged) {
            /* Get MAC Config MAC */
            HAL_ETH_GetMACConfig(&EthHandle, &MACConf);
            MACConf.DuplexMode = duplex;
            MACConf.Speed = speed;
            HAL_ETH_SetMACConfig(&EthHandle, &MACConf);
            HAL_ETH_Start(&EthHandle);
            netif_set_up(netif);
            netif_set_link_up(netif);
        }
    }
}

extern "C" void HAL_ETH_RxAllocateCallback(uint8_t **buff) {
    struct pbuf_custom *p = reinterpret_cast<struct pbuf_custom *>(LWIP_MEMPOOL_ALLOC(RX_POOL));
    if (p) {
        /* Get the buff from the struct pbuf address. */
        *buff = (uint8_t *)p + offsetof(RxBuff_t, buff);
        p->custom_free_function = pbuf_free_custom;
        /* Initialize the struct pbuf.
         * This must be performed whenever a buffer's allocated because it may be
         * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
        pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUFFER_SIZE);
    } else {
        RxAllocStatus = RX_ALLOC_ERROR;
        *buff = nullptr;
    }
}

extern "C" void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff, uint16_t Length) {
    struct pbuf **ppStart = (struct pbuf **)pStart;
    struct pbuf **ppEnd = (struct pbuf **)pEnd;
    struct pbuf *p = NULL;

    /* Get the struct pbuf from the buff address. */
    p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
    p->next = NULL;
    p->tot_len = 0;
    p->len = Length;

    /* Chain the buffer. */
    if (!*ppStart) {
        /* The first buffer of the packet. */
        *ppStart = p;
    } else {
        /* Chain the buffer to the end of the packet. */
        (*ppEnd)->next = p;
    }
    *ppEnd = p;

    /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
     * set to its own length, plus the length of all the following pbufs in the chain. */
    for (p = *ppStart; p != NULL; p = p->next) {
        p->tot_len += Length;
    }

    /* Invalidate data cache because Rx DMA's writing to physical memory makes it stale. */
    SCB_InvalidateDCache_by_Addr((uint32_t *)buff, Length);
}

extern "C" void HAL_ETH_TxFreeCallback(uint32_t *buff) {
    pbuf_free((struct pbuf *)buff);
}
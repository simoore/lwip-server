#include <cstdint>
#include <cstring>
#include <span>

#include "etl/vector.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

#include "lwipserver/drivers/Lan8742.h"
#include "lwipserver/stm32h7/Base.h"
#include "lwipserver/stm32h7/TxDescriptor.h"

/*****************************************************************************/
/********** CONSTANTS ********************************************************/
/*****************************************************************************/

static constexpr uint32_t sEthTxDescCount = ETH_TX_DESC_CNT;

#if LWIP_NETIF_HOSTNAME
static constexpr bool sUseHostName = true;
#else
static constexpr bool sUseHostName = false;
#endif

#define ETH_RX_BUFFER_SIZE 1000U
#define ETH_RX_BUFFER_CNT 12U

#define TX_DESC_ATTRIBUTES static __attribute__((section(".TxDecripSection")))
#define RX_DESC_ATTRIBUTES static __attribute__((section(".RxDecripSection")))

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

static_assert(sizeof(ETH_DMADescTypeDef) == sizeof(lwipserver::stm32h7::TxDescriptor));

using PacketBuf = struct pbuf;

/// The RX buffer type consists of a 32 byte aligned buffer and a pbuf struct to use with LwIP.
typedef struct {
    struct pbuf_custom pbuf_custom;
    uint8_t buff[(ETH_RX_BUFFER_SIZE + 31) & ~31] __ALIGNED(32);
} RxBuff_t;

/// The ethernet RX descriptors.
RX_DESC_ATTRIBUTES ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]; 

/// The ethernet TX descriptors.
TX_DESC_ATTRIBUTES lwipserver::stm32h7::TxDescriptor sTxDescriptors[sEthTxDescCount];

/// Memory Pool Declaration
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");
extern __attribute__((section(".Rx_PoolSection"))) u8_t memp_memory_RX_POOL_base[];

/// Indicates if RX Buffers are available in the pool.
static bool sRxBuffersAvailable = true;

/// Global Ethernet handle
ETH_HandleTypeDef EthHandle;

/// Keeps track of index in the sTxDescriptors array of the location of the next available Tx Buffer.
static uint32_t sAvailableIndex = 0;

/// Keeps track of the number of available TX Descriptors that the application can use.
static uint32_t sAvaliableSize = sEthTxDescCount;

/// Location of the next Tx Desccriptor to check if packet transmission is finished to free pbuf.
static uint32_t sUnavailableTxDescIndex = 0;

/// The driver for the ethernet PHY.
static lwipserver::drivers::Lan8742 sLan8742;

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

/*****************************************************************************/
/********** FUNCTION DECLARATIONS ********************************************/
/*****************************************************************************/

void pbuf_free_custom(struct pbuf *p);
void ethernet_link_check_state(struct netif *netif);

/*****************************************************************************/
/********** FUNCTION DEFINITIONS *********************************************/
/*****************************************************************************/

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
    EthHandle.Init.TxDesc = reinterpret_cast<ETH_DMADescTypeDef *>(sTxDescriptors);
    EthHandle.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

    // configure ethernet peripheral (GPIOs, clocks, MAC, DMA)
    HAL_ETH_Init(&EthHandle);

    // set MAC hardware address length
    netif->hwaddr_len = ETH_HWADDR_LEN;

    // set MAC hardware address
    netif->hwaddr[0] = ETH_MAC_ADDR0;
    netif->hwaddr[1] = ETH_MAC_ADDR1;
    netif->hwaddr[2] = ETH_MAC_ADDR2;
    netif->hwaddr[3] = ETH_MAC_ADDR3;
    netif->hwaddr[4] = ETH_MAC_ADDR4;
    netif->hwaddr[5] = ETH_MAC_ADDR5;

    // maximum transfer unit
    netif->mtu = ETH_MAX_PAYLOAD;

    // device capabilities
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    // Initialize the RX POOL
    LWIP_MEMPOOL_INIT(RX_POOL);

    HAL_ETH_SetMDIOClockRange(&EthHandle);
    sLan8742.init<lwipserver::stm32h7::Base, Ether>();

    ethernet_link_check_state(netif);
}

/// Takes the a chained packet buffer from LwIP and sets up an ethernet transaction to send it.
///
/// @param netif 
///     The network interface handle from LwIP. Unused.
/// @param p 
///     The packet buffer to transmit.
/// @return 
///     ERR_IF if there are not enough TX descriptors available for the packet. ERR_OK otherwise.
static err_t lowLevelOutput(struct netif *netif, PacketBuf *p) {
    static_cast<void>(netif);
    
    // Analyse the received pbufs to determine the number of descriptors and the total payload size.
    etl::vector<PacketBuf *, sEthTxDescCount> bufs;
    uint32_t payloadLength = 0;

    for (PacketBuf *q = p; q != nullptr; q = q->next) {
        if (bufs.full()) {
            return ERR_IF;
        }
        bufs.push_back(q);
        payloadLength += q->len;
    }

    // Determine if we have enough descriptors for the packet. Each descriptor supports two buffers.
    if (bufs.size() > sAvaliableSize * 2) {
        return ERR_IF;
    }

    // When this function returns, LwIP is going to free the buffer. Incrementing the reference count prevents this 
    // from happening while the ETH DMA is reading the buffer. We must free it later.
    printf("ref %lu\n", reinterpret_cast<uint32_t>(p));
    pbuf_ref(p);

    // Fill in the DMA desriptors.
    uint32_t bufIndex = 0;

    // Retrieve the buffer information for the next descriptor.
    auto createSpan = [&bufIndex, &bufs](void) -> std::span<uint8_t> {
        if (bufIndex < bufs.size()) {
            PacketBuf *q = bufs[bufIndex];
            auto buf = std::span<uint8_t>(reinterpret_cast<uint8_t *>(q->payload), q->len);
            bufIndex += 1;
            return buf;
        }
        return std::span<uint8_t>();
    };

    while (bufIndex < bufs.size()) {

        lwipserver::stm32h7::TxDescriptor &desc = sTxDescriptors[sAvailableIndex];
        if (bufIndex == 0) {
            desc.setFirstDescriptor(); 
        }
        auto buf1 = createSpan();
        auto buf2 = createSpan();
        desc.set(buf1, buf2, payloadLength);

        if (bufIndex == bufs.size()) {
            desc.setLastDescriptor();
            desc.setAppData(reinterpret_cast<uint32_t>(p), 0);
        } else {
            desc.setAppData(0, 0);
        }
        __DMB();
        desc.setOwned();

        // We keep track of where the next available descriptor should be and how many descriptors are available.
        sAvailableIndex += 1;
        if (sAvailableIndex == sEthTxDescCount) {
            sAvailableIndex = 0;
        }
        sAvaliableSize -= 1;
        printf("use: %lu %lu\n", sAvaliableSize, sAvailableIndex);
    }

    // Ensure completion of descriptor preparation before transmission start.
    __DMB();

    // Start transmission, issue a poll command to Tx DMA by writing address of next immediate free descriptor.
    WRITE_REG(ETH->DMACTDTPR, reinterpret_cast<uint32_t>(sTxDescriptors + sAvailableIndex));
    printf("Tail pointer %lu\n", ETH->DMACTDTPR);

    return ERR_OK;
}


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

    if (sRxBuffersAvailable) {
        HAL_ETH_ReadData(&EthHandle, (void **)&p);
    }
    return p;
}

/// Check for TX packets that have been transmitted and free their pbufs. 
static void releaseTxBuffers(void) {
    while (true) {

        // Only check when sUnavailableTxDescIndex is behind sAvailableIndex.
        if (sAvailableIndex == sUnavailableTxDescIndex) {
            return;  
        }

        lwipserver::stm32h7::TxDescriptor &desc = sTxDescriptors[sUnavailableTxDescIndex];
        // TODO: When I get to sUnavailableTxDescIndex 19, the DMA never de-asserts the own index and buffers are never
        // maybe for some reason the tail pointer wasn't updated. If I set the number of descriptors to 16, the issue
        // doesn't appear. So its either the location of the descriptor or the number of descriptors. This issue is 
        // associated with the fact that I cannot define the number of descriptors different from ETH_TX_DESC_CNT 
        // without issues.
        if (desc.ownedByDMA()) {
            return;
        }

        if (desc.getAppData0()) {
            pbuf_free(reinterpret_cast<PacketBuf *>(desc.getAppData0()));
            printf("Free %lu\n", desc.getAppData0());
        }
        sAvaliableSize += 1;
        sUnavailableTxDescIndex += 1;
        if (sUnavailableTxDescIndex == sEthTxDescCount) {
            sUnavailableTxDescIndex = 0;
        }
        printf("release: %lu %lu\n", sAvaliableSize, sUnavailableTxDescIndex);
    }
}

/// ethernetif_input is called periodically to read from the network interface and pass packets to the TCP/IP stack.
/// It uses the function low_level_input() that handles the actual reception of bytes from the network interface. 
/// Then the type of the received packet is determined and the appropriate input function is called.
///
/// @param netif 
///     The lwip network interface structure for this ethernetif
void ethernetif_input(struct netif *netif) {
    releaseTxBuffers();
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

/// Should be called at the beginning of the program to set up the network interface. It calls the function 
/// low_level_init() to do the actual setup of the hardware. This function should be passed as a parameter to 
/// netif_add().
///
/// @param netif 
///     The lwip network interface structure for this ethernetif.
/// @return 
///     ERR_OK if the interface is initialized, ERR_ARG if netif is nullptr.
err_t ethernetif_init(struct netif *netif) {

    if (netif == nullptr) {
        return ERR_ARG;
    }

#if LWIP_NETIF_HOSTNAME
        netif->hostname = "lwipserver.local";
#endif

    netif->name[0] = 'l';
    netif->name[1] = 's';
    netif->output = etharp_output;

    low_level_init(netif);
    netif->linkoutput = lowLevelOutput;

    return ERR_OK;
}

/// Free for RX packet buffer.
/// @param pbuf 
///     Packet buffer to be freed
void pbuf_free_custom(struct pbuf *p) {
    struct pbuf_custom *custom_pbuf = (struct pbuf_custom *)p;
    LWIP_MEMPOOL_FREE(RX_POOL, custom_pbuf);
    if (!sRxBuffersAvailable) {
        sRxBuffersAvailable = true;
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
        sRxBuffersAvailable = false;
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

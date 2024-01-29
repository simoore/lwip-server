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

static constexpr uint32_t sEthTxDescCount = 16;

#if LWIP_NETIF_HOSTNAME
static constexpr bool sUseHostName = true;
#else
static constexpr bool sUseHostName = false;
#endif

static constexpr bool sUseHAL = false;

#define IFNAME0 's'
#define IFNAME1 't'

#define ETH_DMA_TRANSMIT_TIMEOUT (20U)

#define ETH_RX_BUFFER_SIZE 1000U
#define ETH_RX_BUFFER_CNT 12U
#define ETH_TX_BUFFER_MAX ((ETH_TX_DESC_CNT)*2U)

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

using PacketBuf = struct pbuf;

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
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT];// __attribute__((section(".TxDecripSection")));

static lwipserver::stm32h7::TxDescriptor sTxDescriptors[sEthTxDescCount] __attribute__((section(".TxDecripSection")));

// Memory Pool Declaration
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");
__attribute__((section(".Rx_PoolSection"))) extern u8_t memp_memory_RX_POOL_base[];

/* Variable Definitions */
static uint8_t RxAllocStatus;

/* Global Ethernet handle*/
ETH_HandleTypeDef EthHandle;
ETH_TxPacketConfig TxConfig;

/// Keeps track of index in the sTxDescriptors array of the location of the next available Tx Buffer.
static uint32_t sAvailableIndex = 0;

/// Keeps track of the number of available TX Descriptors that the application can use.
static uint32_t sAvaliableSize = sEthTxDescCount;

/// Location of the next Tx Desccriptor to check if packet transmission is finished to free pbuf.
static uint32_t sUnavailableTxDescIndex = 0;

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

// /**
//  * @brief In this function, the hardware should be initialized.
//  * Called from ethernetif_init().
//  *
//  * @param netif the already initialized lwip network interface structure
//  *        for this ethernetif
//  */
// static void low_level_init(struct netif *netif) {
//     uint8_t macaddress[6] = {ETH_MAC_ADDR0, ETH_MAC_ADDR1, ETH_MAC_ADDR2, ETH_MAC_ADDR3, ETH_MAC_ADDR4, ETH_MAC_ADDR5};

//     EthHandle.Instance = ETH;
//     EthHandle.Init.MACAddr = macaddress;
//     EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
//     EthHandle.Init.RxDesc = DMARxDscrTab;
//     EthHandle.Init.TxDesc = DMATxDscrTab;
//     EthHandle.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

//     /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
//     HAL_ETH_Init(&EthHandle);

//     /* set MAC hardware address length */
//     netif->hwaddr_len = ETH_HWADDR_LEN;

//     /* set MAC hardware address */
//     netif->hwaddr[0] = ETH_MAC_ADDR0;
//     netif->hwaddr[1] = ETH_MAC_ADDR1;
//     netif->hwaddr[2] = ETH_MAC_ADDR2;
//     netif->hwaddr[3] = ETH_MAC_ADDR3;
//     netif->hwaddr[4] = ETH_MAC_ADDR4;
//     netif->hwaddr[5] = ETH_MAC_ADDR5;

//     /* maximum transfer unit */
//     netif->mtu = ETH_MAX_PAYLOAD;

//     /* device capabilities */
//     /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
//     netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

//     /* Initialize the RX POOL */
//     LWIP_MEMPOOL_INIT(RX_POOL);

//     /* Set Tx packet config common parameters */
//     memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
//     TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
//     TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
//     TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

//     HAL_ETH_SetMDIOClockRange(&EthHandle);
//     sLan8742.init<lwipserver::stm32h7::Base, Ether>();

//     ethernet_link_check_state(netif);
// }

static void lowLevelInit(struct netif *netif) {

    uint8_t macaddress[6] = {ETH_MAC_ADDR0, ETH_MAC_ADDR1, ETH_MAC_ADDR2, ETH_MAC_ADDR3, ETH_MAC_ADDR4, ETH_MAC_ADDR5};

    EthHandle.Instance = ETH;
    EthHandle.Init.MACAddr = macaddress;
    EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
    EthHandle.Init.RxDesc = DMARxDscrTab;
    EthHandle.Init.TxDesc = DMATxDscrTab;
    EthHandle.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

    // configure ethernet peripheral (GPIOs, clocks, MAC, DMA)
    HAL_ETH_Init(&EthHandle);

    /////////////////////////////////

    #if 0
    // Init the low level hardware : GPIO, CLOCK, NVIC
    HAL_ETH_MspInit(nullptr);

    // Used to select either an MII or RMII interface to the PHY.
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    HAL_SYSCFG_ETHInterfaceSelect(SYSCFG_ETH_RMII);

    // Ethernet Software reset, resets all MAC subsystem internal registers and logic
    // After reset all the registers holds their respective reset values.
    SET_BIT(ETH->DMAMR, ETH_DMAMR_SWR);
    uint32_t tickstart = HAL_GetTick();
    while (READ_BIT(ETH->DMAMR, ETH_DMAMR_SWR) > 0U) {
        if (HAL_GetTick() > (ETH_SWRESET_TIMEOUT + tickstart)) {
            while (true);
        }
    }

    /*------------------ MDIO CSR Clock Range Configuration --------------------*/
    ETH_MAC_MDIO_ClkConfig(&EthHandle);

    // MAC LPI 1US Tic Counter Configuration
    WRITE_REG(ETH->MAC1USTCR, (((uint32_t)HAL_RCC_GetHCLKFreq() / ETH_MAC_US_TICK) - 1U));

    /*------------------ MAC, MTL and DMA default Configuration ----------------*/
    ETH_MACDMAConfig(heth);

    // We have added a 64 bit spacing between descriptors for optional application data.
    MODIFY_REG(ETH->DMACCR, ETH_DMACCR_DSL, ETH_DMACCR_DSL_64BIT);

    // Specify the length of the receive buffers.
    MODIFY_REG(ETH->DMACRCR, ETH_DMACRCR_RBSZ, ((ETH_RX_BUFFER_SIZE) << 1));
    static_assert(ETH_RX_BUFFER_SIZE % 4 == 0, "RX Buffer length must be multiple of 4");
    
    //ETH_DMATxDescListInit(heth);
    ETH_DMARxDescListInit(heth);

    // Set ethernet MAC address.
    heth->Instance->MACA0HR = (static_cast<uint32_t>(ETH_MAC_ADDR5) << 8) | static_cast<uint32_t>(ETH_MAC_ADDR4);
    heth->Instance->MACA0LR = 
        (static_cast<uint32_t>(ETH_MAC_ADDR3) << 24) | (static_cast<uint32_t>(ETH_MAC_ADDR2) << 16) | 
        (static_cast<uint32_t>(ETH_MAC_ADDR1) << 8)  | (static_cast<uint32_t>(ETH_MAC_ADDR0));
    #endif

    // Set Transmit Descriptor Ring Length
    WRITE_REG(ETH->DMACTDRLR, (sEthTxDescCount - 1));

    // Set Transmit Descriptor List Address
    WRITE_REG(ETH->DMACTDLAR, reinterpret_cast<uint32_t>(sTxDescriptors));

    // Set Transmit Descriptor Tail pointer
    WRITE_REG(ETH->DMACTDTPR, reinterpret_cast<uint32_t>(sTxDescriptors));

    /////////////////////////////////

    netif->hwaddr_len = ETH_HWADDR_LEN;
    netif->hwaddr[0] = ETH_MAC_ADDR0;
    netif->hwaddr[1] = ETH_MAC_ADDR1;
    netif->hwaddr[2] = ETH_MAC_ADDR2;
    netif->hwaddr[3] = ETH_MAC_ADDR3;
    netif->hwaddr[4] = ETH_MAC_ADDR4;
    netif->hwaddr[5] = ETH_MAC_ADDR5;
    netif->mtu = ETH_MAX_PAYLOAD;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    // Initialize the RX POOL
    LWIP_MEMPOOL_INIT(RX_POOL);

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
// static err_t low_level_output(struct netif *netif, struct pbuf *p) {
//     uint32_t i = 0U;
//     struct pbuf *q = nullptr;
//     err_t errval = ERR_OK;
//     ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT] = {0};

//     memset(Txbuffer, 0, ETH_TX_DESC_CNT * sizeof(ETH_BufferTypeDef));

//     for (q = p; q != nullptr; q = q->next) {
//         if (i >= ETH_TX_DESC_CNT)
//             return ERR_IF;

//         Txbuffer[i].buffer = reinterpret_cast<uint8_t *>(q->payload);
//         Txbuffer[i].len = q->len;

//         if (i > 0) {
//             Txbuffer[i - 1].next = &Txbuffer[i];
//         }

//         if (q->next == NULL) {
//             Txbuffer[i].next = nullptr;
//         }

//         i++;
//     }

//     TxConfig.Length = p->tot_len;
//     TxConfig.TxBuffer = Txbuffer;
//     TxConfig.pData = p;

//     HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);

//     return errval;
// }

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
    __DSB();

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

    if (RxAllocStatus == RX_ALLOC_OK) {
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
        // doesn't appear. So its either the location of the descriptor or the number of descriptors.
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
    if constexpr (!sUseHAL) {
        releaseTxBuffers();
    }
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

    if constexpr (sUseHAL) {
        //low_level_init(netif);
        //netif->linkoutput = low_level_output;
    } else {
        lowLevelInit(netif);
        netif->linkoutput = lowLevelOutput;
    }
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

#include "Stm32h7Eth.h"

/// Ethernet Rx DMA Descriptors.
Stm32h7Eth::RxDescriptors DMARxDscrTab __attribute__((section(".RxDecripSection"))); 

/// Ethernet Tx DMA Descriptors.
Stm32h7Eth::TxDescriptors DMATxDscrTab __attribute__((section(".TxDecripSection")));   

LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");

void Stm32h7Eth::init(const IEth::Config &cfg) {

    // Enable GPIO and ETH clocks.
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();
    
    // Configure pins on port A.
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin =  GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL ; 
    GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // Configure pins on port G.
    GPIO_InitStructure.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    // Configure pins on port C.
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5; 
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	

    // Configure the ETH peripheral.
    mEthHandle.Instance = ETH;  
    mEthHandle.Init.MACAddr = cfg.macAddr;
    mEthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
    mEthHandle.Init.RxDesc = sDMARxDscrTab;
    mEthHandle.Init.TxDesc = sDMATxDscrTab;
    mEthHandle.Init.RxBuffLen = cfg.rxBufferSize;
    HAL_ETH_Init(&mEthHandle);
    
    HAL_ETH_RegisterRxAllocateCallback(&mEthHandle, &rxAllocateCallback);

    // Initialize the RX pool.
    LWIP_MEMPOOL_INIT(RX_POOL);
    /*
    mMempDesc = {.size(), \
    (num), \
    memp_memory_ ## name ## _base, \
    &memp_tab_ ## name \.num = sRxBufferCount, }
    memp_init_pool(&mMempDesc);
    LWIP_MEMPOOL_INIT(RX_POOL);
    */

    // Enable interrupts.
    HAL_NVIC_SetPriority(ETH_IRQn, 12, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
}

bool Stm32h7Eth::isTransmitSuspended() const {
    const auto dmatxdesc = reinterpret_cast<const ETH_DMADescTypeDef *>(
        mEthHandle.TxDescList.TxDesc[mEthHandle.TxDescList.CurTxDesc]
    );
    return ((dmatxdesc->DESC3 & ETH_DMATXNDESCWBF_OWN) != RESET);
}

void Stm32h7Eth::startEthernet(DuplexMode duplexMode, Speed speed) {
    ETH_MACConfigTypeDef MACConf;
    HAL_ETH_GetMACConfig(&mEthHandle, &MACConf); 
    MACConf.DuplexMode = (duplexMode == DuplexMode::FullDuplex) ? ETH_FULLDUPLEX_MODE : ETH_HALFDUPLEX_MODE;
    MACConf.Speed = (speed == Speed::Hundred) ? ETH_SPEED_100M : ETH_SPEED_10M;
    HAL_ETH_SetMACConfig(&mEthHandle, &MACConf);
    HAL_ETH_Start_IT(&mEthHandle);
};
#ifndef STM32H7_ETH_H
#define STM32H7_ETH_H

#include "stm32h7xx_hal.h"
#include "IEth.h"

class Stm32h7Eth: public IEth {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// Number of TX descriptors. This value is defined in the HAL conf.
    static constexpr uint32_t sEthTxDescCnt{ETH_TX_DESC_CNT};

    /// Number of RX descriptors. This value is defined in the HAL conf.
    static constexpr uint32_t sEthRxDescCnt{ETH_RX_DESC_CNT};

    /// A TX descriptor contains two buffer pointers.
    static constexpr uint32_t sTxBufferCount{2 * sEthTxDescCnt};

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// The list of desriptors used by the RX DMA stream.
    using RxDescriptors = ETH_DMADescTypeDef[sEthRxDescCnt];

    /// The list of descriptors used by the TX DMA stream.
    using TxDescriptors = ETH_DMADescTypeDef[sEthTxDescCnt];

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Remove the copy constructor to enforce singleton design pattern.
    Stm32h7Eth(Stm32h7Eth const &) = delete;

    /// Remove the copy operator to enforce the singleton design pattern.
    void operator=(Stm32h7Eth const &) = delete;

    /// Encaptulates and returns the singleton instance of this ethernet interface.
    ///
    /// @return
    ///     The ethernet interface instance.
    static Stm32h7Eth &getInstance() {
        static Stm32h7Eth instance;
        return instance;
    }

    /// Here we initialize hardware peripheral in the STM32H7 processor.
    ///
    /// RMII Interface:
    /// RX_CLK  --------------> PA1  
    /// TXD0    --------------> PG12  
    /// TXD1    --------------> PG13  
    /// RXD0    --------------> PC4
    /// RXD1    --------------> PC5
    /// TX_EN   --------------> PG11 
    /// RX_DV   --------------> PA7    
    /// MDC     --------------> PC1
    /// MDIO    --------------> PA2
    ///
    /// @param cfg
    ///     Configuration of the the ethernet peripheral for this application.
    void init(const Config &cfg) override;

    /// We detect suspension by examining the OWN bit of the last TX descriptor we have set for transmission.
    /// When it returns to the application there is no more data to transmit.
    ///
    /// @return
    ///     True if the ETH peripherla is suspended because it sent all of its data.
    IEth::Status &status() const override {

    }

    void startEthernet(DuplexMode duplexMode, Speed speed) override;

    void stopEthernet() override {
        HAL_ETH_Stop(&mEthHandle);
    }

    /// This function checks if a complete ethernet packet is ready for the application to process.
    /// 
    /// @return
    ///     A pointer to the new RX packet. nullptr if no packet is ready. Call from main loop only.
    Packet *readData() override {
        Packet *p;
        // HAL_ETH_ReadData allocates a packet buffer and writes the pointer to `p` above.
        // 1. It checks that the supplied double pointer doesn't point to null.
        // 2. It checks that the peripheral is running.
        // 3. The structure heth->RxDescList is of type ETH_RxDescListTypeDef
        //      a. This is DMA Receive Descriptors Wrapper structure definition
        //      b. RxDescList.RxDescIdx is the current descriptor index. It is the next descriptor the ETH DMA 
        //          writes too and it hasn't been processed and returned to the application.
        //      c. RxDescList.RxBuildDescCnt is the number of descriptors assigned to the application from the last
        //          packet. These should returned to the ETH peripheral after each new packet arrives.
        // 4. We then loop through the chain of RX descriptors until either: 
        //      a. The DMA module still owns the descriptos thus the whole packet hasn't arrived
        //      b. We have run out of unbuilt descriptors, ie. descriptors not associated with the application.
        //      c. We have found the last descriptor in the packet (success)
        //      d. Descriptor register DESC3, bit ETH_DMARXNDESCWBF_OWN indicates if DMA or application own the 
        //          descriptor.
        // 5. For each descriptor in the loop, we perform the following checks
        //      a. Check if this is a receive context descriptor. This type of descriptor provides additional 
        //          information about the last received packet and contains no data. Here it provides a timestamp.
        //      b. Then we check if the current descriptor is the first and last desciptor that makes up the packet
        //          and set various fields associated with these two cases.
        //      c. Then call the RxLinkCallback to assign the buffer to the application packet buffer or descriptor
        //          list.
        // 6. We update_descriptors, that is return the descriptors used by the last packet for use by the driver.
        // 7. If the end of a packet is found, a non-null value is returned.
        // 8. HAL will save the state of the last call to this function so you don't have to search from the 
        //      beginning of the packet again.
        HAL_ETH_ReadData(&mEthHandle, reinterpret_cast<void **>(&p));
        return p;
    }

    bool writeData(Packet &packet) override {

        // We have 3 descriptor formats in this system, there is out generic IEth::Descriptor format that is passed
        // to this function, there is as HAL desciptor format (ETH_BufferTypeDef) that is passed into HAL functions
        // and there is a format required by the STM32 ETH peripheral. prepareTxConfig() converts our generic 
        // descriptor format to the HAL descriptor format.
        //
        // TODO: Is there an easy way to skip the HAL format and go straight from generic to STM32 descriptor format.
        const bool ok = prepareTxConfig(packet);
        if (!ok) {
            return false;
        }

        // You can call this multiple times. As long as there are enough descriptors available to queue the data.
        //
        // What is going on here:
        // 1. mEthHandle.gState is checked, it just checks the ETH peripheral is not busy or in error.
        // 2. Then ETH_Prepare_Tx_Descriptors() is called. It sets fields in the descriptors some of which are:
        //  a. TDES0 contains the buffer 1 address BUF1AP.
        //  b. TDES1 contains the buffer 2 address BUF2AP.
        //  c. TDES2 contains:
        //      * IOC - if an interrupt should be raised after this buffer is transmitted.
        //      * B2L & B1L - the lengths of the buffers.
        //  c. TDES3 contains:
        //      * A number of flags about whether to alter ETH, IP, or TCP header fields
        //      * Whether the buffer is owned by the application or DMA.
        //      * Flags for the first and last buffer of a packet.
        //      * Context or normal descriptor.
        //      * Full length of packet.
        // 3. Transmission is started or extended by writing to the location of the tail pointer. The tail pointer 
        //  points to the next descriptor still owned by the application. The register is DMACTXDTPR - DMA Channel TX 
        //  Descriptor tail pointer.
        //
        // If there are not enough descriptors available for all packet buffers in the chain, the tail pointer is
        // not updated, and the HAL tail pointer (mEthHandle.TxDescList->CurTxDesc) is not updated.
        HAL_StatusTypeDef status = HAL_ETH_Transmit_IT(&mEthHandle, &mTxConfig);
        if (status == HAL_ERROR) {
            if (mEthHandle.ErrorCode & HAL_ETH_ERROR_BUSY) {
                printf("Stm32h7Eth::output, not enough available STM32 descriptors.\n");
            } else {
                printf("Stm32h7Eth::output, unknown error %d.\n", mEthHandle.ErrorCode);
            }
            return false;
        }
        return ok;
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// This function is called during HAL_ETH_ReadData when the HAL wants to connect an RX DMA buffer to application's
    /// descriptor format.
    ///
    /// @param pStart
    ///     Points to the first application descriptor in the ethernet packet. If it is nullptr, then the supplied 
    ///     buffer is the the first buffer in the packet and pStart should be assigned by this function.
    /// @param pEnd
    ///     Was designed to point to the last application descriptor in a chain. We aren't using this parameter in this
    ///     module. You could use it to store arbitary data since it is not actually used by the HAL layer.
    /// @param buffer
    ///     The buffer to be added to an application descriptor.
    /// @param size
    ///     The size of the buffer.
    void rxLinkCallback(void **pStart, void **pEnd, uint8_t *buffer, uint16_t size) {
        static_cast<void>(pEnd);

        // If *pStart is null, then this is the start of a new ethernet packet. Set *pStart and clear the descriptor
        // list in mRxPacket.
        if (!(*pStart)) {
            *pStart = &mRxPacket;
            mRxPacket.descriptorList.clear();
        }

        // Add the new buffer to list of desriptors.
        mRxPacket.descriptorList.emplace_back(Descriptor{ .buffer = buffer, .size = size });

        // Invalidate data cache because Rx DMA's writing to physical memory makes it stale.
        SCB_InvalidateDCache_by_Addr(reinterpret_cast<uint32_t *>(buffer), size);
    }

    /// Connect the chained packet buffers to the ETH_TxPacketConfig object expected by the HAL library. If there
    /// are not enough remaining buffers, ERR_IF is returned. txConfig is not required after the transmission is
    /// started.
    ///
    /// @param descriptors
    ///     List of generic descriptors from the EthernetInterface module.
    /// @return
    ///     If there was enough HAL TX descriptors available.
    bool prepareTxConfig(Packet &packet) {
        
        // If we run out of TX buffers for this transaction, return failure.
        if (packet.descriptorList.size() > sTxBufferCount) {
            printf("Stm32h7Eth::prepareTxConfig, not enough ETH_BufferTypeDef's for TX.");
            return false;
        }

        uint16_t totalSize = 0;
        for (uint16_t i = 0; i < packet.descriptorList.size(); i++) {

            // Link the single packet buffer to a TX buffer.
            mTxBuffers[i].buffer = packet.descriptorList[i].buffer;
            mTxBuffers[i].len = packet.descriptorList[i].size;
            mTxBuffers[i].next = nullptr;
            totalSize += packet.descriptorList[i].size;

            // Chain the TX buffers together.
            if (i > 0) {
                mTxBuffers[i-1].next = &mTxBuffers[i];
            }

            // Cleaning cache isn't necessary if the tx buffers are in a not-cacheable MPU region.
            #if ETH_TX_BUFFERS_ARE_CACHED
            uint8_t *dataStart = q->payload;
            uint8_t *lineStart = (uint8_t *)((uint32_t)dataStart & ~31);
            SCB_CleanDCache_by_Addr((uint32_t *)lineStart, q->len + (dataStart - lineStart));
            #endif
        }

        mTxConfig = { .Length = totalSize, .TxBuffer = mTxBuffers };
        return true;
    }

    /// When we build a descriptor, the HAL will ask for a buffer from the application. Here we link the buffer to
    /// our memory management approach and return the pointer
    void rxAllocateCallback(uint8_t **buff) {
        *buff = callback();
    }

            /**
     * @brief  Ethernet Rx Transfer completed callback
     * @param  heth: ETH handle
     * @retval None
     */
    void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
    {
        portBASE_TYPE taskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(EthIfThread, &taskWoken);
        portYIELD_FROM_ISR(taskWoken);
    }

    #if ETH_TX_QUEUE_ENABLE
    /**
     * @brief  Ethernet Tx Transfer completed callback
     * @param  heth: ETH handle
     * @retval None
     */
    void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
    {
    portBASE_TYPE taskWoken = pdFALSE;
    xSemaphoreGiveFromISR(TxPktSemaphore, &taskWoken);
    portYIELD_FROM_ISR(taskWoken);
    }
    #endif /* ETH_TX_QUEUE_ENABLE */

    /**
     * @brief  Ethernet DMA transfer error callback
     * @param  heth: ETH handle
     * @retval None
     */
    void HAL_ETH_DMAErrorCallback(ETH_HandleTypeDef *heth)
    {
        /* If a receive buffer unavailable error occurred, signal the
        * ethernetif_input task to call HAL_ETH_GetRxDataBuffer to rebuild the
        * Rx Descriptors and restart receive. */
        if (heth->DMAErrorCode & ETH_DMACSR_RBU)
        {
            portBASE_TYPE taskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(EthIfThread, &taskWoken);
            portYIELD_FROM_ISR(taskWoken);
        }
    }

    void HAL_ETH_RxAllocateCallback(uint8_t **buff) {
        
    }

    void HAL_ETH_TxFreeCallback(uint32_t * buff) {
        pbuf_free((struct pbuf *)buff);
    }

    static void staticRxLinkCallback(void **pStart, void **pEnd, uint8_t *buffer, uint16_t size) {
        getInstance().rxLinkCallback(pStart, pEnd, buffer, size);
    }

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// Ethernet Rx DMA Descriptors.
    static RxDescriptors sDMARxDscrTab; 

    /// Ethernet Tx DMA Descriptors.
    static TxDescriptors sDMATxDscrTab;

    /// Ethernet HAL handle.
    ETH_HandleTypeDef mEthHandle;

    ETH_TxPacketConfig mTxConfig;

    ETH_BufferTypeDef mTxBuffers[sTxBufferCount];

    #define ETH_RX_BUFFER_CNT 12
    
    LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "Zero-copy RX PBUF pool");

    /// The packet structure that is fille in when the ETH peripheral has a complete packet ready for the application
    /// to process.
    Packet mRxPacket;

    /*
    // RX Buffer memory pool descri
#define LWIP_MEMPOOL_DECLARE(name,num,size,desc) \
  LWIP_DECLARE_MEMORY_ALIGNED(memp_memory_ ## name ## _base, ((num) * (MEMP_SIZE + MEMP_ALIGN_SIZE(size)))); \
    \
  LWIP_MEMPOOL_DECLARE_STATS_INSTANCE(memp_stats_ ## name) \
    \
  static struct memp *memp_tab_ ## name; \
    \
  const struct memp_desc memp_ ## name = { \
    DECLARE_LWIP_MEMPOOL_DESC(desc) \
    LWIP_MEMPOOL_DECLARE_STATS_REFERENCE(memp_stats_ ## name) \
    LWIP_MEM_ALIGN_SIZE(size), \
    (num), \
    memp_memory_ ## name ## _base, \
    &memp_tab_ ## name \
  };
*/

}; // class Stm32h7Eth

#endif // STM32H7_ETH_H
#include <algorithm>
#include <array>
#include <cerrno>
#include <span>
#include <string_view>
#include <unistd.h>

#include "stm32h7xx_hal.h"

#include "lwipserver/freertos/Mutex.h"
#include "lwipserver/freertos/StreamBuffer.h"
#include "lwipserver/utils/DmaRxBuffer.h"

namespace lwipserver::stm32h7 {

/*************************************************************************/
/********** TYPES ********************************************************/
/*************************************************************************/

/// Configuration settings for the debung UART module.
struct DebugUartConfig {
    uint32_t baud{1500000};                 ///< Baud rate used by the debug UART peripheral.
    uint32_t blockingTxTimeout_ms{1000};    ///< The timeout setting for blocking UART operations.
    uint32_t txWait_ms{20};                 ///< 
    bool useStreamRx{true};                 ///< Whether the RX channel uses blocking for a streaming DMA method.
    bool useStreamTx{true};                 ///< If true, sends _write data to DMA buffer (needs to be serviced).
};

/*************************************************************************/
/********** VARIABLES ****************************************************/
/*************************************************************************/

static UART_HandleTypeDef sHuart;   ///< STM32 HAL UART handle.
static DMA_HandleTypeDef sHdmaRx;   ///< STM32 HAL DMA handle for UART RX
static DMA_HandleTypeDef sHdmaTx;   ///< STM32 HAL DMA handle for UART TX
static DebugUartConfig sConfig;     ///< Settings for this module.
static freertos::StreamBuffer<1024> sStreamBuffer(64);
static freertos::Mutex sMutex;

/// Use to flag to a call of _read that if there is a UART error that is should return.
static bool sUartError{false};

/// We align the RX buffer because cache coherency options occur on 32 byte align chunks.
static __attribute((section(".dmamem1"), aligned(32))) DmaRxBuffer sRxBuffer;

/// This is the DMA buffer for UART TX. Make sure the buffers are in memory locations accessible by the DMA peripheral 
/// they are streaming to.
static __attribute((section(".dmamem1"), aligned(32))) std::array<uint8_t, 64> sTxBuffer1;

static_assert((sRxBuffer.sBufferSize % 32) == 0, 
    "Buffer size should be a multiple of 32 bytes for STM32H7 cache coherency operations.");
static_assert((sTxBuffer1.size() % 32) == 0, 
    "Buffer size should be a multiple of 32 bytes for STM32H7 cache coherency operations.");

/*************************************************************************/
/********** EXPORTED FUNCTIONS *******************************************/
/*************************************************************************/

/// Copy a message to the debug UART DMA buffer.
///
/// @param data 
///     The buffer containing the data to write to the transmit buffer.
size_t debugUartTx(std::span<const uint8_t> data) {
    sMutex.lock(freertos::sWaitForever);
    size_t size = sStreamBuffer.send(data, sConfig.txWait_ms);
    sMutex.unlock();
    return size;
}

/// Copy a message to the debug UART DMA buffer.
///
/// @param data 
///     The buffer containing the data to write to the transmit buffer.
size_t debugUartTx(std::string_view data) {
    return debugUartTx({reinterpret_cast<const uint8_t *>(data.data()), data.size()});
}

/// This function should go into the main loop so the DMA buffers can be periodically checked for new data.
void debugUartService() {
    if (sHuart.gState != HAL_UART_STATE_BUSY_TX) {
        uint16_t size = sStreamBuffer.receive(sTxBuffer1, 0);
        if (size > 0) {
            SCB_CleanDCache_by_Addr(reinterpret_cast<uint32_t *>(sTxBuffer1.data()), sTxBuffer1.size());
            HAL_UART_Transmit_DMA(&sHuart, sTxBuffer1.data(), size);
        }
    }
}

/// On the UART error interrupt, we abort the current transactions, place debug message into the TX buffer, and
/// flag to the blocking _read() function that you should terminate with error.
static void uartErroHandler(UART_HandleTypeDef *huart) {
    HAL_UART_Abort(&sHuart);
    sUartError = true;
    switch (huart->hdmarx->ErrorCode) {
    case HAL_DMA_ERROR_NONE:
        break;
    case HAL_DMA_ERROR_TE:
        // DMA peripherals can only read and write to certain memory locations. If you attempt to read and write
        // from/to an unsupported memory address, you will get a transfer error.
        debugUartTx("RX DMA Transfer Error\n");
        break;
    default:
        debugUartTx("RX DMA Error\n");
        break;
    }
    if (huart->ErrorCode) {
        debugUartTx("UART Error\n");
    }
    if (!huart->ErrorCode && !huart->hdmarx->ErrorCode) {
        debugUartTx("UART Error with no ErrorCode\n");
    }
    if (sConfig.useStreamRx) {
        HAL_UART_Receive_DMA(&sHuart, sRxBuffer.buffer(), sRxBuffer.sBufferSize);
    }
}

/// The initialization of this module requires the following steps:
/// 1. Configuration of UART GPIO
/// 2. Configuration of UART peripheral
/// 3. Configuration of DMA peripherals.
/// 4. Enabling interrupts.
void debugUartInit() {

    ///////////////////////////////////////////////////////////////////////////
    // GPIO Initialization
    ///////////////////////////////////////////////////////////////////////////

    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin       = GPIO_PIN_8;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin       = GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    ///////////////////////////////////////////////////////////////////////////
    // UART Initialization
    ///////////////////////////////////////////////////////////////////////////

    __HAL_RCC_USART3_CLK_ENABLE();
    sHuart.Instance                     = USART3;
    sHuart.Init.BaudRate                = sConfig.baud;
    sHuart.Init.WordLength              = UART_WORDLENGTH_8B;
    sHuart.Init.StopBits                = UART_STOPBITS_1;
    sHuart.Init.Parity                  = UART_PARITY_NONE;
    sHuart.Init.HwFlowCtl               = UART_HWCONTROL_NONE;
    sHuart.Init.Mode                    = UART_MODE_TX_RX;
    sHuart.Init.OverSampling            = UART_OVERSAMPLING_16;
    sHuart.Init.OneBitSampling          = UART_ONE_BIT_SAMPLE_DISABLE;
    sHuart.Init.ClockPrescaler          = UART_PRESCALER_DIV1;
    sHuart.AdvancedInit.AdvFeatureInit  = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&sHuart) != HAL_OK) {
        while (true);
    }

    ///////////////////////////////////////////////////////////////////////////
    // DMA Initialization
    ///////////////////////////////////////////////////////////////////////////

    __HAL_RCC_DMA1_CLK_ENABLE();

    sHdmaTx.Instance                 = DMA1_Stream0;
    sHdmaTx.Init.Request             = DMA_REQUEST_USART3_TX;
    sHdmaTx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    sHdmaTx.Init.PeriphInc           = DMA_PINC_DISABLE;
    sHdmaTx.Init.MemInc              = DMA_MINC_ENABLE;
    sHdmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    sHdmaTx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    sHdmaTx.Init.Mode                = DMA_NORMAL;
    sHdmaTx.Init.Priority            = DMA_PRIORITY_LOW;
    sHdmaTx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    sHdmaTx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    sHdmaTx.Init.MemBurst            = DMA_MBURST_SINGLE;
    sHdmaTx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    if (HAL_DMA_Init(&sHdmaTx) != HAL_OK) {
        while (true);
    }
    __HAL_LINKDMA(&sHuart, hdmatx, sHdmaTx);

    sHdmaRx.Instance                 = DMA1_Stream1;
    sHdmaRx.Init.Request             = DMA_REQUEST_USART3_RX;
    sHdmaRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    sHdmaRx.Init.PeriphInc           = DMA_PINC_DISABLE;
    sHdmaRx.Init.MemInc              = DMA_MINC_ENABLE;
    sHdmaRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    sHdmaRx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    sHdmaRx.Init.Mode                = sConfig.useStreamRx ? DMA_CIRCULAR : DMA_NORMAL;
    sHdmaRx.Init.Priority            = DMA_PRIORITY_HIGH;
    sHdmaRx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    sHdmaRx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    sHdmaRx.Init.MemBurst            = DMA_MBURST_SINGLE;
    sHdmaRx.Init.PeriphBurst         = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&sHdmaRx) != HAL_OK) {
        while (true);
    }
    __HAL_LINKDMA(&sHuart, hdmarx, sHdmaRx);

    ///////////////////////////////////////////////////////////////////////////
    // Interrupt Initialization
    ///////////////////////////////////////////////////////////////////////////

    // Preempt priority indicates if an interrupt should execute if another lower priority interrupt is executing.
    // Subpriority only considers which interrupt to execute next if two interrupts of the same priority are waiting
    // to execute.
    HAL_UART_RegisterCallback(&sHuart, HAL_UART_ERROR_CB_ID, uartErroHandler);

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 15, 2);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    HAL_NVIC_SetPriority(USART3_IRQn, 15, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    if (sConfig.useStreamRx) {
        HAL_UART_Receive_DMA(&sHuart, sRxBuffer.buffer(), sRxBuffer.sBufferSize);
    }
}

} // namespace lwipserver::stm32h7

/*************************************************************************/
/********** INTERRUPT HANDLERS *******************************************/
/*************************************************************************/

extern "C" void USART3_IRQHandler() {
    HAL_UART_IRQHandler(&lwipserver::stm32h7::sHuart);
}

extern "C" void DMA1_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&lwipserver::stm32h7::sHdmaTx);
}

extern "C" void DMA1_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&lwipserver::stm32h7::sHdmaRx);
}

/*************************************************************************/
/********** SYSCALLS *****************************************************/
/*************************************************************************/

/// newlib syscall to write data to a file. We use this to send data from printf statements. 
///
/// @param fd
///     The file descriptor that the data is being written to. We only support stdout and stderr files.
/// @param data 
///     The buffer containing the data to write to the transmit buffer.
/// @param size
///     The size of the data to write to the buffer.
extern "C" int _write(int fd, char *data, int size) {
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        if (lwipserver::stm32h7::sConfig.useStreamTx) {
            return lwipserver::stm32h7::debugUartTx(
                {reinterpret_cast<const uint8_t *>(data), static_cast<size_t>(size)});
        } else {
            HAL_UART_Transmit(&lwipserver::stm32h7::sHuart, reinterpret_cast<const uint8_t *>(data), 
                static_cast<uint16_t>(size), lwipserver::stm32h7::sConfig.blockingTxTimeout_ms);
            return size; 
        }
    }
    errno = EBADF;
    return -1;
}

/// Initiates a read from the UART RX stream. This function will block execution until the RX idle interrupt is 
/// executed. So only call scanf() from the main loop to prevent interrupts from being blocked.
///
/// @param fd
///     The file descriptor number. We only support stdin.
/// @param data
///     The buffer to read into.
/// @param size
///     The size of the buffer.
extern "C" int _read(int fd, char *data, int size) {
    if (fd == STDIN_FILENO) {
        if (lwipserver::stm32h7::sConfig.useStreamRx) {
            lwipserver::stm32h7::sUartError = false;
            while (true) {
                const int localCounter = __HAL_DMA_GET_COUNTER(&lwipserver::stm32h7::sHdmaRx);
                lwipserver::stm32h7::sRxBuffer.updatePointers(localCounter);
                SCB_InvalidateDCache_by_Addr(lwipserver::stm32h7::sRxBuffer.buffer(), 
                    lwipserver::stm32h7::sRxBuffer.sBufferSize);
                int copySize = lwipserver::stm32h7::sRxBuffer.copy(reinterpret_cast<uint8_t *>(data), size);
                if (copySize > 0) {
                    return copySize;
                }
                if (lwipserver::stm32h7::sUartError) {
                    errno = EIO;
                    return -1;
                }
            }
        } else {
            const auto hstatus = HAL_UART_Receive(&lwipserver::stm32h7::sHuart, 
                lwipserver::stm32h7::sRxBuffer.buffer(), 1, HAL_MAX_DELAY);
            if (hstatus == HAL_OK) {
                std::copy_n(lwipserver::stm32h7::sRxBuffer.buffer(), 1, data);
                return 1;
            } else {
                return EIO;
            }
        }
    }
    errno = EBADF;
    return -1;
}
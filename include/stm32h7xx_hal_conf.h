#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/********** HAL MODULES & OPTIONS ********************************************/
/*****************************************************************************/

#define HAL_MODULE_ENABLED  

// #define HAL_ADC_MODULE_ENABLED   
// #define HAL_FDCAN_MODULE_ENABLED   
// #define HAL_CEC_MODULE_ENABLED   
// #define HAL_COMP_MODULE_ENABLED   
// #define HAL_CRC_MODULE_ENABLED   
// #define HAL_CRYP_MODULE_ENABLED   
// #define HAL_DAC_MODULE_ENABLED   
// #define HAL_DCMI_MODULE_ENABLED   
// #define HAL_DMA2D_MODULE_ENABLED   
#define HAL_ETH_MODULE_ENABLED 
// #define HAL_NAND_MODULE_ENABLED   
// #define HAL_NOR_MODULE_ENABLED   
// #define HAL_SRAM_MODULE_ENABLED   
// #define HAL_SDRAM_MODULE_ENABLED   
// #define HAL_HASH_MODULE_ENABLED   
// #define HAL_HRTIM_MODULE_ENABLED   
// #define HAL_HSEM_MODULE_ENABLED   
// #define HAL_JPEG_MODULE_ENABLED   
// #define HAL_OPAMP_MODULE_ENABLED   
// #define HAL_I2S_MODULE_ENABLED   
// #define HAL_SMBUS_MODULE_ENABLED   
// #define HAL_IWDG_MODULE_ENABLED   
// #define HAL_LPTIM_MODULE_ENABLED   
// #define HAL_LTDC_MODULE_ENABLED   
// #define HAL_QSPI_MODULE_ENABLED   
// #define HAL_RNG_MODULE_ENABLED   
// #define HAL_RTC_MODULE_ENABLED   
// #define HAL_SAI_MODULE_ENABLED   
// #define HAL_SD_MODULE_ENABLED   
// #define HAL_MMC_MODULE_ENABLED   
// #define HAL_SPDIFRX_MODULE_ENABLED   
// #define HAL_SPI_MODULE_ENABLED   
// #define HAL_SWPMI_MODULE_ENABLED   
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_USART_MODULE_ENABLED 
// #define HAL_IRDA_MODULE_ENABLED   
// #define HAL_SMARTCARD_MODULE_ENABLED   
// #define HAL_WWDG_MODULE_ENABLED   
// #define HAL_PCD_MODULE_ENABLED   
// #define HAL_HCD_MODULE_ENABLED   
// #define HAL_DFSDM_MODULE_ENABLED   
// #define HAL_DSI_MODULE_ENABLED   
// #define HAL_MDIOS_MODULE_ENABLED   
#define HAL_EXTI_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_MDMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_HSEM_MODULE_ENABLED

#define USE_HAL_UART_REGISTER_CALLBACKS 1

/*****************************************************************************/
/********** CLOCK VALUES *****************************************************/
/*****************************************************************************/

// These clock frequencies and startup times are used in the HAL library to configure frequencies for many functions.
// They must be correct for correct functioning of the processor. The values below are the clock frequencies 
// of the oscillators on the NUCLEO-H732ZI2 board. Timeouts are in ms, frequencies are in Hz.

#if !defined  (HSE_VALUE) 
#define HSE_VALUE    ((uint32_t)8000000) 
#endif

#if !defined  (HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT    ((uint32_t)100U)  
#endif

#if !defined  (CSI_VALUE)
#define CSI_VALUE    ((uint32_t)4000000)
#endif 
   
#if !defined  (HSI_VALUE)
#define HSI_VALUE    ((uint32_t)64000000)
#endif

#if !defined  (LSE_VALUE)
#define LSE_VALUE    ((uint32_t)32768U)
#endif 

#if !defined  (LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT    ((uint32_t)5000U) 
#endif

#if !defined  (EXTERNAL_CLOCK_VALUE)
#define EXTERNAL_CLOCK_VALUE    12288000U
#endif

/*****************************************************************************/
/********** SYSTEM CONFIGURATION *********************************************/
/*****************************************************************************/

/// Supply voltage in mV.
#define VDD_VALUE ((uint32_t)3300U)

/// The priority of the tick timer interrupt (0 is the highest).
#define TICK_INT_PRIORITY ((uint32_t)0U)

#define USE_RTOS 0U
#define USE_SD_TRANSCEIVER 1U              
#define USE_SPI_CRC 0U

/*****************************************************************************/
/********** ETHERNET PERIPHERAL CONFIGURATION ********************************/
/*****************************************************************************/

#define ETH_TX_DESC_CNT ((uint32_t)4U)     
#define ETH_RX_DESC_CNT ((uint32_t)4U)      

// MAC Address of the ethernet peripheral. This can be any value so long as it is unique on the local ethernet network
// it is connected to. For this project, just randomly select a number. For products were this constraint is more
// critical, you can buy MAC address IC's to read of to maximize the change of having a globally unique MAC address.

#define ETH_MAC_ADDR0    ((uint8_t)0x00)
#define ETH_MAC_ADDR1    ((uint8_t)0x80)
#define ETH_MAC_ADDR2    ((uint8_t)0xE1)
#define ETH_MAC_ADDR3    ((uint8_t)0x00)
#define ETH_MAC_ADDR4    ((uint8_t)0x00)
#define ETH_MAC_ADDR5    ((uint8_t)0x00)

/*****************************************************************************/
/********** HAL MODULE INCLUDES **********************************************/
/*****************************************************************************/

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32h7xx_hal_rcc.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
#include "stm32h7xx_hal_exti.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32h7xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32h7xx_hal_dma.h"
#endif

#ifdef HAL_HASH_MODULE_ENABLED
#include "stm32h7xx_hal_hash.h"
#endif

#ifdef HAL_DCMI_MODULE_ENABLED
#include "stm32h7xx_hal_dcmi.h"
#endif

#ifdef HAL_DMA2D_MODULE_ENABLED
#include "stm32h7xx_hal_dma2d.h"
#endif

#ifdef HAL_DFSDM_MODULE_ENABLED
#include "stm32h7xx_hal_dfsdm.h"
#endif

#ifdef HAL_ETH_MODULE_ENABLED
#include "stm32h7xx_hal_eth.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32h7xx_hal_cortex.h"
#endif

#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32h7xx_hal_adc.h"
#endif

#ifdef HAL_FDCAN_MODULE_ENABLED
#include "stm32h7xx_hal_fdcan.h"
#endif

#ifdef HAL_CEC_MODULE_ENABLED
#include "stm32h7xx_hal_cec.h"
#endif

#ifdef HAL_COMP_MODULE_ENABLED
#include "stm32h7xx_hal_comp.h"
#endif

#ifdef HAL_CRC_MODULE_ENABLED
#include "stm32h7xx_hal_crc.h"
#endif

#ifdef HAL_CRYP_MODULE_ENABLED
#include "stm32h7xx_hal_cryp.h" 
#endif

#ifdef HAL_DAC_MODULE_ENABLED
#include "stm32h7xx_hal_dac.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32h7xx_hal_flash.h"
#endif

#ifdef HAL_HRTIM_MODULE_ENABLED
#include "stm32h7xx_hal_hrtim.h"
#endif

#ifdef HAL_HSEM_MODULE_ENABLED
#include "stm32h7xx_hal_hsem.h"
#endif

#ifdef HAL_SRAM_MODULE_ENABLED
#include "stm32h7xx_hal_sram.h"
#endif

#ifdef HAL_NOR_MODULE_ENABLED
#include "stm32h7xx_hal_nor.h"
#endif

#ifdef HAL_NAND_MODULE_ENABLED
#include "stm32h7xx_hal_nand.h"
#endif
      
#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32h7xx_hal_i2c.h"
#endif

#ifdef HAL_I2S_MODULE_ENABLED
#include "stm32h7xx_hal_i2s.h"
#endif

#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32h7xx_hal_iwdg.h"
#endif

#ifdef HAL_JPEG_MODULE_ENABLED
#include "stm32h7xx_hal_jpeg.h"
#endif

#ifdef HAL_MDIOS_MODULE_ENABLED
#include "stm32h7xx_hal_mdios.h"
#endif

#ifdef HAL_MDMA_MODULE_ENABLED
#include "stm32h7xx_hal_mdma.h"
#endif
   
#ifdef HAL_LPTIM_MODULE_ENABLED
#include "stm32h7xx_hal_lptim.h"
#endif

#ifdef HAL_LTDC_MODULE_ENABLED
#include "stm32h7xx_hal_ltdc.h"
#endif

#ifdef HAL_OPAMP_MODULE_ENABLED
#include "stm32h7xx_hal_opamp.h"
#endif
   
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32h7xx_hal_pwr.h"
#endif

#ifdef HAL_QSPI_MODULE_ENABLED
#include "stm32h7xx_hal_qspi.h"
#endif
   
#ifdef HAL_RNG_MODULE_ENABLED
#include "stm32h7xx_hal_rng.h"
#endif

#ifdef HAL_RTC_MODULE_ENABLED
#include "stm32h7xx_hal_rtc.h"
#endif

#ifdef HAL_SAI_MODULE_ENABLED
#include "stm32h7xx_hal_sai.h"
#endif

#ifdef HAL_SD_MODULE_ENABLED
#include "stm32h7xx_hal_sd.h"
#endif

#ifdef HAL_MMC_MODULE_ENABLED
#include "stm32h7xx_hal_mmc.h"
#endif

#ifdef HAL_SDRAM_MODULE_ENABLED
#include "stm32h7xx_hal_sdram.h"
#endif
   
#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32h7xx_hal_spi.h"
#endif

#ifdef HAL_SPDIFRX_MODULE_ENABLED
#include "stm32h7xx_hal_spdifrx.h"
#endif

#ifdef HAL_SWPMI_MODULE_ENABLED
#include "stm32h7xx_hal_swpmi.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32h7xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
#include "stm32h7xx_hal_uart.h"
#endif

#ifdef HAL_USART_MODULE_ENABLED
#include "stm32h7xx_hal_usart.h"
#endif

#ifdef HAL_IRDA_MODULE_ENABLED
#include "stm32h7xx_hal_irda.h"
#endif

#ifdef HAL_SMARTCARD_MODULE_ENABLED
#include "stm32h7xx_hal_smartcard.h"
#endif 

#ifdef HAL_SMBUS_MODULE_ENABLED
#include "stm32h7xx_hal_smbus.h"
#endif 

#ifdef HAL_WWDG_MODULE_ENABLED
#include "stm32h7xx_hal_wwdg.h"
#endif 
   
#ifdef HAL_PCD_MODULE_ENABLED
#include "stm32h7xx_hal_pcd.h"
#endif 

#ifdef HAL_HCD_MODULE_ENABLED
#include "stm32h7xx_hal_hcd.h"
#endif
   
/*****************************************************************************/
/********** HAL ASSERT PARAM NOT USED ****************************************/
/*****************************************************************************/

#define assert_param(expr) ((void)0)

#ifdef __cplusplus
}
#endif

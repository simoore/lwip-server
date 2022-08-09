#include "stm32h7xx.h"

#if !defined  (HSE_VALUE)
    #define HSE_VALUE    ((uint32_t)25000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (CSI_VALUE)
    #define CSI_VALUE    ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* CSI_VALUE */

#if !defined  (HSI_VALUE)
    #define HSI_VALUE    ((uint32_t)64000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/************************* Miscellaneous Configuration ************************/
/*!< Uncomment the following line if you need to use external SRAM or SDRAM mounted
     on EVAL board as data memory  */
/*#define DATA_IN_ExtSRAM */
/*#define DATA_IN_ExtSDRAM*/

/*!< Uncomment the following line if you need to use initialized data in D2 domain SRAM  */
/* #define DATA_IN_D2_SRAM */

/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */
/* #define VECT_TAB_SRAM */

/// Vector Table base offset field.
static constexpr uint32_t sVectTabOffset{0};
static_assert(sVectTabOffset % 0x200 == 0, "Vector table offset must be a multiple of 0x200");

// These variable is updated in three ways:
// 1) by calling CMSIS function SystemCoreClockUpdate()
// 2) by calling HAL API function HAL_RCC_GetHCLKFreq()
// 3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency 
// Note: If you HAL_RCC_ClockConfig() this function to configure the system clock; then there is no need to call the 2 
// first functions listed above, since SystemCoreClock variable is updated automatically.

extern "C" {
/// This global variable is used to store the frequency of the system core clock and is used in calculations in 
/// the STM32H7 HAL library.
uint32_t SystemCoreClock = 64000000;

/// This is the clock speed of... It is used in the STM32 HAL library.
uint32_t SystemD2Clock = 64000000;

/// ...
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
}

/// Initialize the FPU setting, vector table location and external memory configuration.
void System_Init() {
    #if defined (DATA_IN_D2_SRAM)
    __IO uint32_t tmpreg;
    #endif /* DATA_IN_D2_SRAM */

    /* FPU settings ------------------------------------------------------------*/
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));  /* set CP10 and CP11 Full Access */
    #endif

    // We set these registers to default values because we can power on in a random state and these registers at least 
    // should be consistent before executing the application.
    RCC->CR |= RCC_CR_HSION;
    RCC->CFGR = 0x00000000;
    RCC->CR &= 0xEAF6ED7FU;
    RCC->D1CFGR = 0x00000000;
    RCC->D2CFGR = 0x00000000;
    RCC->D3CFGR = 0x00000000;
    RCC->PLLCKSELR = 0x00000000;
    RCC->PLLCFGR = 0x00000000;
    RCC->PLL1DIVR = 0x00000000;
    RCC->PLL1FRACR = 0x00000000;
    RCC->PLL2DIVR = 0x00000000;
    RCC->PLL2FRACR = 0x00000000;
    RCC->PLL3DIVR = 0x00000000;
    RCC->PLL3FRACR = 0x00000000;
    RCC->CR &= 0xFFFBFFFFU;
    RCC->CIER = 0x00000000;

    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    if((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U) {
    /* if stm32h7 revY*/
    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
        *((__IO uint32_t*)0x51008108) = 0x00000001U;
    }

    #if defined (DATA_IN_D2_SRAM)
    /* in case of initialized data in D2 SRAM , enable the D2 SRAM clock */
    RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN | RCC_AHB2ENR_D2SRAM3EN);
    tmpreg = RCC->AHB2ENR;
    (void) tmpreg;
    #endif /* DATA_IN_D2_SRAM */

    /*
    * Disable the FMC bank1 (enabled after reset).
    * This, prevents CPU speculation access on this bank which blocks the use of FMC during
    * 24us. During this time the others FMC master (such as LTDC) cannot use it!
    */
    FMC_Bank1_R->BTCR[0] = 0x000030D2;
    #if defined (DATA_IN_ExtSRAM) || defined (DATA_IN_ExtSDRAM)
    SystemInit_ExtMemCtl(); 
    #endif /* DATA_IN_ExtSRAM || DATA_IN_ExtSDRAM */

    /* Configure the Vector Table location add offset address ------------------*/
    #ifdef VECT_TAB_SRAM
    SCB->VTOR = D1_AXISRAM_BASE  | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal SRAM */
    #else
    SCB->VTOR = FLASH_BANK1_BASE | sVectTabOffset;       /* Vector Table Relocation in Internal FLASH */
    #endif  

    // Enable SoC ram memories.
    __HAL_RCC_D2SRAM1_CLK_ENABLE();
    __HAL_RCC_D2SRAM2_CLK_ENABLE();
}

#include "FreeRTOS.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_cortex.h"
#include "task.h"

// This is defined in the Cortex M7 port of FreeRTOS
extern "C" void xPortSysTickHandler(void);

/// Called during HAL init to initialize the tick timer.
extern "C" HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0);
    return HAL_OK;
}

/// Disable the tick increment by disabling the interrupts..
extern "C" void HAL_SuspendTick(void) {
    LL_SYSTICK_DisableIT();
    
}

/// Resume tick increments by enabling interrupts.
extern "C" void HAL_ResumeTick(void) {
    LL_SYSTICK_EnableIT();
}

/// The systick interrupt handler increments the tick in the STM32 HAL and FreeRTOS.
extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}

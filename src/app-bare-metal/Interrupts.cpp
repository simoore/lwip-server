#include "stm32h7xx_hal.h"

// This function handles System service call via SWI instruction.
extern "C" void SVC_Handler(void) {}

// This function handles Pendable request for system service.
extern "C" void PendSV_Handler(void) {}

// This function handles System tick timer.
extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
}
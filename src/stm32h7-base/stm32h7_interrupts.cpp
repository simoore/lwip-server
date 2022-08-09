#include "stm32h7xx_hal.h"


extern "C" void NMI_Handler(void) {

}

// This function handles Hard fault interrupt.
extern "C" void HardFault_Handler(void) {
    while (true);
}

// This function handles Memory management fault.
extern "C" void MemManage_Handler(void) {
    while (true);
}

// This function handles Pre-fetch fault, memory access fault.
extern "C" void BusFault_Handler(void) {
    while (true);
}

// This function handles Undefined instruction or illegal state.
extern "C" void UsageFault_Handler(void) {
    while (true);
}

// This function handles System service call via SWI instruction.
extern "C" void SVC_Handler(void) {}

// This function handles Debug monitor.
extern "C" void DebugMon_Handler(void) {}

// This function handles Pendable request for system service.
extern "C" void PendSV_Handler(void) {}

// This function handles System tick timer.
extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
}

#include "FreeRTOS.h"
#include "task.h"

extern "C" void xPortSysTickHandler(void);

extern "C" void SysTick_Handler(void) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}


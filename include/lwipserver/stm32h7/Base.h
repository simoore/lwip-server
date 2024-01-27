#pragma once

#include "stm32h7xx_hal.h"

#include "lwipserver/stm32h7/DebugUart.h"

namespace lwipserver::stm32h7 {

class Base final {
public:

    static void init() {
        mpuConfig();
        cpuCacheEnable();
        HAL_Init();
        systemClockConfig();
        debugUartInit();
    }

    static void cpuCacheEnable() {
        SCB_EnableICache();
        SCB_EnableDCache();
    }

    static void mpuConfig();
    
    static void systemClockConfig();

    /// Blocking call that returns after the wait time.
    ///
    /// @param time_ms
    ///     The time to wait.
    static void wait(uint32_t time_ms) {
        HAL_Delay(time_ms);
    }

    /// The system tick is a counter that increments every millisecond. 
    ///
    /// @return
    ///     The value of the system tick.
    static uint32_t tick(void) {
        return HAL_GetTick();
    }

    /// Service any system components that need to be executed routinely in the main loop/
    static void service(void) {
        debugUartService();
    }

    /// Transmits a debug message. Non-blocking operation.
    ///
    /// @param data
    ///     The buffer containing the data.
    /// @param size
    ///     The size of the buffer.
    static void debug(const char *data, uint32_t size) {
        debugUartTx(data, size);
    }

};

} // namespace lwipserver::stm32h7

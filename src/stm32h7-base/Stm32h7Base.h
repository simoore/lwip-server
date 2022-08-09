#ifndef STM32H7_BASE_H
#define STM32H7_BASE_H

#include "IBase.h"
#include "stm32h7xx_hal.h"
#include "stm32h7_debug_uart.h"

class Stm32h7Base: public IBase {
public:

    void init() {
        mpuConfig();
        cpuCacheEnable();
        HAL_Init();
        systemClockConfig();
        debugUartInit();
    }

    void cpuCacheEnable() {
        SCB_EnableICache();
        SCB_EnableDCache();
    }

    void mpuConfig();
    
    void systemClockConfig();

    /// Blocking call that returns after the wait time.
    ///
    /// @param time_ms
    ///     The time to wait.
    void wait(uint32_t time_ms) const override {
        HAL_Delay(time_ms);
    }

    /// The system tick is a counter that increments every millisecond. 
    ///
    /// @return
    ///     The value of the system tick.
    uint32_t tick() const override {
        return HAL_GetTick();
    }

    /// Service any system components that need to be executed routinely in the main loop/
    void service() override {
        debugUartService();
    }

    /// Transmits a debug message. Non-blocking operation.
    ///
    /// @param data
    ///     The buffer containing the data.
    /// @param size
    ///     The size of the buffer.
    void debug(const char *data, int size) const override {
        debugUartTx(data, size);
    }

}; // Stm32h7Base

#endif // STM32H7_BASE_H
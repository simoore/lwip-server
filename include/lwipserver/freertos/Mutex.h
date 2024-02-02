#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace lwipserver::freertos {

static constexpr uint32_t sWaitForever = 0xFFFFFFFF;

class Mutex {
public:

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Initializes the semaphore.
    Mutex() {
        mHandle = xSemaphoreCreateMutexStatic(&mStaticSemaphore);
    }

    /// Attempts to lock mutex.
    /// 
    /// @param wait_ms 
    ///     Maximum time to wait.
    /// @return 
    ///     True if mutex was locked, otherwise false on timeout.
    bool lock(uint32_t wait_ms) {
        return xSemaphoreTake(mHandle, pdMS_TO_TICKS(wait_ms)) == pdTRUE;
    }

    /// Unlocks the mutex. 
    /// 
    /// @return 
    ///     True if successful, false on error.
    bool unlock(void) {
        return xSemaphoreGive(mHandle) == pdTRUE;
    }

private:

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    /// FreeRTOS mutex handle.
    SemaphoreHandle_t mHandle;

    /// Control block for the semaphore.
    StaticSemaphore_t mStaticSemaphore;

};

} // namespace lwipserver::freertos
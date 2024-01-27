#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include "FreeRTOS.h"
#include "task.h"

namespace lwipserver::freertos {

/// Encaptulates a FreeRTOS task.
///
/// @tparam stackSize 
///     This is the size of the stack in words.
template <size_t stackSize>
class OsTask final {
public:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Stores parameters for the task for when it is created.
    OsTask(std::string_view name) : mName(name) {}

    /// Creates and launches the task.
    void create(std::function<void(void)> taskFunction, uint32_t priority) {
        mTaskFunction = taskFunction;
        mTaskHandle = xTaskCreateStatic(taskWrapper, mName.data(), stackSize, this, priority, mStack, &mStaticTask);
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// The static function that freertos uses to launch the task.
    static void taskWrapper(void *arg) {
        auto &task = *reinterpret_cast<OsTask<stackSize> *>(arg);
        if (task.mTaskFunction) {
            task.mTaskFunction();
        }
        vTaskDelete(nullptr);
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    /// The freertos task handle.
    TaskHandle_t mTaskHandle = nullptr;

    /// The name of the task.
    std::string_view mName;

    /// Memory allocation for the stack.
    StackType_t mStack[stackSize];

    /// The control block for the task.
    StaticTask_t mStaticTask;

    /// The task function.
    std::function<void(void)> mTaskFunction;

};

} // namespace lwipserver::freertos
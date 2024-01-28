#include "lwipserver/freertos/OsTask.h"
#include "lwipserver/network/NetworkRTOS.h"
#include "lwipserver/network/TcpEchoServer.h"
#include "lwipserver/stm32h7/Base.h"

using namespace lwipserver;

/*****************************************************************************/
/********** SYSTEM COMPONENTS ************************************************/
/*****************************************************************************/

static constexpr uint32_t sHeartbeatInterval{2000};

network::Network sNetwork;
TcpEchoServer sTcpEchoServer;

/*****************************************************************************/
/********** MAIN AND TASKS ***************************************************/
/*****************************************************************************/

int main() {
    stm32h7::Base::init();
    sNetwork.init();
    sTcpEchoServer.init();
    vTaskStartScheduler();
    while (true) {}
}

/*****************************************************************************/
/********** IDLE TASK HOOK AND MEMORY FOR NETWORK STACK **********************/
/*****************************************************************************/

/// We run the network stack as part of the idle task.
void vApplicationIdleHook(void) {
    stm32h7::Base::service();
    sNetwork.lwipThread<stm32h7::Base>();
}

/// Because we are running the network stack as part of the idle task, we need a bigger idle task stack. The default
/// implementation of this function in the FreeRTOS kernel sets the stack size to configMINIMAL_STACK_SIZE. While this 
/// config is not used for anything else in the kernel and could be used to set a larger, a number of ports use this to 
/// set stack sizes for other tasks so I rather not increase it.
///
/// @param ppxIdleTaskTCBBuffer
///     Return a reference to the control block for the task.
/// @param ppxIdleTaskStackBuffer 
///     Returns the location of the stack for a task.
/// @param pulIdleTaskStackSize 
///     Returns the size of the stack in words.
void vApplicationGetIdleTaskMemory(
    StaticTask_t **ppxIdleTaskTCBBuffer,
    StackType_t **ppxIdleTaskStackBuffer,
    uint32_t *pulIdleTaskStackSize
) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[network::Network::sLwIPTaskStackSize];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = network::Network::sLwIPTaskStackSize;
}

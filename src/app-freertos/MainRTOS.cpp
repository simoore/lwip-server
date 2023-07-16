#include "NetworkRTOS.h"
#include "Stm32h7Base.h"
#include "TcpEchoServer.h"
#include "FreeRTOS.h"
#include "task.h"

Stm32h7Base sStm32h7Base;
Network sNetwork{sStm32h7Base};
TcpEchoServer sTcpEchoServer;

static constexpr uint32_t sHeartbeatInterval{2000};
static uint16_t sStartTaskStackSize = 4 * configMINIMAL_STACK_SIZE;

// I assume the modules initialized in this thread require the OS scheduler to be running.
static void startThread(void *argument) {
    sNetwork.init();
    sTcpEchoServer.init();
    while (true) {
        vTaskDelete(nullptr);
    }
}

int main() {
    sStm32h7Base.init();

    auto status = xTaskCreate(startThread, "StartTask", sStartTaskStackSize, nullptr, PRIORITY_NORMAL, nullptr);
    if (status == pdFAIL) {
        while (true) {}
    }
    vTaskStartScheduler();

    while (true) {}
}

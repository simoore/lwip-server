/// Compile FreeRTOS or Bare-Metal using the NO_SYS option in lwipopts.h

#include "Clock.h"
#include "MqttClient.h"
#include "Network.h"
#include "Stm32h7Base.h"
#include "TcpEchoServer.h"
#include "task.h"
#include <cstdio>

Stm32h7Base sStm32h7Base;
Network sNetwork{sStm32h7Base};
TcpEchoServer sTcpEchoServer;
MqttClient sMqttClient{sStm32h7Base};
Clock sClock{sMqttClient, sStm32h7Base};

static constexpr uint32_t sHeartbeatInterval{2000};
static uint32_t sLastHeartbeatTick{0};
static uint16_t sStartTaskStackSize = 4 * configMINIMAL_STACK_SIZE;

#if 0
void askForName() {
    static char buf[64];
    sStm32h7Base.wait(500);
    printf("\nYour name:\n");
    sStm32h7Base.wait(500);
    scanf("%s", buf);
    sStm32h7Base.wait(500);git 
    printf("\nHello, %s!\n", buf);
}
#endif

// I assume the modules initialized in this thread require the OS scheduler to be running.
static void startThread(void *argument) {
    sNetwork.init();
    // sTcpEchoServer.init();
    while (true) {
        vTaskDelete(nullptr);
    }
}

int main() {
    sStm32h7Base.init();

    if constexpr (!Network::sUsingRTOS) {
        sNetwork.init();
        sTcpEchoServer.init();
        sMqttClient.init(MqttClient::Config());
        sClock.init();
    } else {
        auto status = xTaskCreate(startThread, "StartTask", sStartTaskStackSize, nullptr, PRIORITY_NORMAL, nullptr);
        if (status == pdFAIL) {
            // TODO: add a global error handler.
            while (true);
        }
        vTaskStartScheduler();
    }

    while (true) {
        if constexpr (!Network::sUsingRTOS) {
            if ((sLastHeartbeatTick + sHeartbeatInterval) < sStm32h7Base.tick()) {
                sLastHeartbeatTick = sStm32h7Base.tick();
                printf("Heartbeat!!!\n");
            }
            sNetwork.service();
            sStm32h7Base.service();
            sMqttClient.service();
            sClock.service();
        }
    }
}

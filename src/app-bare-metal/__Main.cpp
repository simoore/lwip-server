#include <cstdio>

#include "lwipserver/network/MqttClient.h"
#include "lwipserver/network/Network.h"
#include "lwipserver/network/TcpEchoServer.h"
#include "lwipserver/stm32h7/Base.h"
#include "lwipserver/time/Clock.h"

Stm32h7Base sStm32h7Base;
Network sNetwork{sStm32h7Base};
TcpEchoServer sTcpEchoServer;
MqttClient sMqttClient{sStm32h7Base};
Clock sClock{sMqttClient, sStm32h7Base};

static constexpr uint32_t sHeartbeatInterval{2000};
static uint32_t sLastHeartbeatTick{0};

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

int main() {
    sStm32h7Base.init();
    sNetwork.init();
    sTcpEchoServer.init();
    sMqttClient.init(MqttClient::Config());
    sClock.init();
   
    while (true) {
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

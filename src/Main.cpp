#include "Network.h"
#include "Stm32h7Base.h"
#include "TcpEchoServer.h"
#include <cstdio>

Stm32h7Base sStm32h7Base;
Network sNetwork{sStm32h7Base};
TcpEchoServer sTcpEchoServer;

static constexpr uint32_t sHeartbeatInterval{2000};
static uint32_t sLastHeartbeatTick{0};

#if 0
void askForName() {
    static char buf[64];
    sStm32h7Base.wait(500);
    printf("\nYour name:\n");
    sStm32h7Base.wait(500);
    scanf("%s", buf);
    sStm32h7Base.wait(500);
    printf("\nHello, %s!\n", buf);
}
#endif

int main() {
    sStm32h7Base.init();
    sNetwork.init();
    sTcpEchoServer.init();
    while (true) {
        if (sLastHeartbeatTick < (sStm32h7Base.tick() - sHeartbeatInterval)) {
            sLastHeartbeatTick = sStm32h7Base.tick();
            printf("Heartbeat!!!\n");
        }
        sNetwork.service();
        sStm32h7Base.service();
    }
}

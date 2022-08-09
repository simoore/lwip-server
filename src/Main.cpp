#include <cstdio>
// #include "TcpEchoServer.h"
// #include "Network.h"
#include "Stm32h7Base.h"

Stm32h7Base sStm32h7Base;
// Network sNetwork;
// TcpEchoServer sTcpEchoServer;

static char buf[64];

int main() {
    sStm32h7Base.init();
    // sNetwork.init();
    // sTcpEchoServer.init();
    while (1) {
        sStm32h7Base.wait(500);
        printf("\nYour name:\n");
        sStm32h7Base.wait(500);
        scanf("%s", buf);
        sStm32h7Base.wait(500);
        printf("\nHello, %s!\n", buf);
    //     sNetwork.service();
        //sStm32h7Base.service();
    }
}

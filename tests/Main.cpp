#include "gmock/gmock.h"
//#include "lwip/init.h"

using namespace ::testing;

int main(int argc, char** argv) {
    //lwip_init();
    InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
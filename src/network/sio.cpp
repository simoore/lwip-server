#include <cstdint>
#include <cstdio>

#include "lwip/sio.h"

extern "C" void sio_send(uint8_t c, sio_fd_t siostat) {
    printf("sio_send, executed.\n");
}

extern "C" uint32_t sio_tryread(sio_fd_t fd, uint8_t* data, uint32_t len) {
    printf("sio_tryread, executed.\n");
    return 0;
}

extern "C" sio_fd_t sio_open(uint8_t devnum) {
    printf("sio_open, executed.\n");
    return nullptr;
}

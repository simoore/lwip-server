#pragma once

#include "gmock/gmock.h"

class BaseMock {
public:
    MOCK_METHOD((void), wait, (uint32_t), (const));
    MOCK_METHOD((uint32_t), tick, (), (const));
    MOCK_METHOD((void), service, (), ());
    MOCK_METHOD((void), debug, (const char *, int), (const));
};

class BaseMockStatic {
public:

    static inline BaseMock *mock = nullptr;

    static void wait(uint32_t time_ms) {
        mock->wait(time_ms);
    }

    static uint32_t tick(void) {
        return mock->tick();
    }

    static void service(void) {
        mock->service();
    }

    static void debug(const char *data, int size) {
        mock->debug(data, size);
    }
};

#pragma once

#include "gmock/gmock.h"
#include "base/IBase.h"

class BaseMock: public IBase {
public:
    MOCK_METHOD((void), wait, (uint32_t), (const, override));
    MOCK_METHOD((uint32_t), tick, (), (const, override));
    MOCK_METHOD((void), service, (), (override));
    MOCK_METHOD((void), debug, (const char *, int), (const, override));
};

#ifndef BASE_MOCK_H
#define BASE_MOCK_H

#include "gmock/gmock.h"
#include "base/IBase.h"

class BaseMock: public IBase {
public:
    MOCK_METHOD((void), wait, (uint32_t), (const, override));
    MOCK_METHOD((uint32_t), tick, (), (const, override));
};

#endif
#pragma once
    
#include "gmock/gmock.h"

#include "lwipserver/base/IEth.h"

class EthMock {
public:
    MOCK_METHOD((std::pair<bool, uint32_t>), readReg, (uint32_t, uint32_t), ());
    MOCK_METHOD((bool), writeReg, (uint32_t, uint32_t, uint32_t), ());
};

class EthMockStatic {
public:

	static inline EthMock *mock = nullptr;

	static std::pair<bool, uint32_t> readReg(uint32_t devAddr, uint32_t regAddr) {
		return mock->readReg(devAddr, regAddr);
	}
	
	static bool writeReg(uint32_t devAddr, uint32_t regAddr, uint32_t regVal) {
		return mock->writeReg(devAddr, regAddr, regVal);
	}
};

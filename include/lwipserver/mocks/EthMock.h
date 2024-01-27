#ifndef ETH_MOCK_H
#define ETH_MOCK_H
    
#include "gmock/gmock.h"
#include "base/IEth.h"

class EthMock: public IEth {
public:
    MOCK_METHOD((void), init, (const Config &cfg), (override));
	MOCK_METHOD((Status &), status, (), (const, override));
	MOCK_METHOD((void), startEthernet, (DuplexMode duplexMode, Speed speed), (override));
	MOCK_METHOD((void), stopEthernet, (), (override));
	MOCK_METHOD((Packet *), readData, (), (override));
	MOCK_METHOD((bool), writeData, (Packet &packet), (override));
    MOCK_METHOD((std::pair<bool, uint32_t>), readReg, (uint32_t, uint32_t), (override));
    MOCK_METHOD((bool), writeReg, (uint32_t, uint32_t, uint32_t), (override));
};

#endif

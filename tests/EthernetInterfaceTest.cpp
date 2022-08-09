#include <algorithm>
#include <iostream>
#include <vector>
#include "gmock/gmock.h"
#include "EthernetInterface.h"
#include "EthMock.h"
#include "BaseMock.h"
#include "lwip/pbuf.h"

using namespace ::testing;

class EthernetInterfaceTest: public Test {
public:

    static inline const std::vector<uint8_t> sOutputData{{0x8A, 0x9B, 0x71, 0xA9, 0xFE, 0xB8, 0x13, 0x84, 0xD0, 0x37}};

    using PacketBuffer = EthernetInterface::PacketBuffer;

    EthMock mEthMock;
    BaseMock mBaseMock;
    EthernetInterface mEthernet{mBaseMock, mEthMock};
    PacketBuffer *mOutputPacket{nullptr};

    EthernetInterfaceTest() {
        // PBUF_RAM allocates memory to LwIP internal heap. Use this for TX buffers.
        mOutputPacket = pbuf_alloc(PBUF_RAW, 10, PBUF_RAM);
        pbuf_take(mOutputPacket, sOutputData.data(), 10);
    }

    // void initExpectationsWithDevAddr1() {
    //     static constexpr uint32_t addr{1};
    //     EXPECT_CALL(mEthMock, readReg(_, Lan8742::sAddr_SMR))
    //         .WillOnce(Return(std::pair{false, 0}))
    //         .WillOnce(Return(std::pair{true, addr}));
    //     EXPECT_CALL(mEthMock, writeReg(addr, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
    //         .WillOnce(Return(true));
    //     EXPECT_CALL(mBaseMock, tick())
    //         .WillOnce(Return(0));
    //     EXPECT_CALL(mEthMock, readReg(addr, Lan8742::sAddr_BCR))
    //         .WillOnce(Return(std::pair{true, 0}));
    //     EXPECT_CALL(mBaseMock, wait(Lan8742::sInitializationWaitTime_ms))
    //         .Times(1);
    // }

    ~EthernetInterfaceTest() {
        pbuf_free(mOutputPacket);
    }
};

ACTION_P(CheckPacket, expectedData) {
    IEth::Packet &actualPacket = arg0;
    ASSERT_EQ(1U, actualPacket.descriptorList.size());
    ASSERT_TRUE(std::equal(expectedData.begin(), expectedData.end(), actualPacket.descriptorList[0].buffer));
}

TEST_F(EthernetInterfaceTest, SendOutputDataTest) {
    IEth::Packet actualPacket;
    EXPECT_CALL(mEthMock, writeData(_))
        .WillOnce(DoAll(CheckPacket(sOutputData), Return(true)));
    err_t err = mEthernet.output(mOutputPacket);
    ASSERT_EQ(err, ERR_OK);
}

TEST_F(EthernetInterfaceTest, ServiceTest) {
    mEthernet.service();
}
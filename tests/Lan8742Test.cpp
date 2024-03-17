#include "gmock/gmock.h"

#include "lwipserver/drivers/Lan8742.h"
#include "lwipserver/mocks/BaseMock.h"
#include "lwipserver/mocks/EthMock.h"

using namespace ::testing;
using namespace lwipserver::drivers;

class Lan8742Test: public Test {
public:
    EthMock mEthMock;
    BaseMock mBaseMock;
    Lan8742 mPhy;

    Lan8742Test() {
        BaseMockStatic::mock = &mBaseMock;
        EthMockStatic::mock = &mEthMock;
    }

    void initExpectationsWithDevAddr1() {
        static constexpr uint32_t addr{1};
        EXPECT_CALL(mEthMock, readReg(_, Lan8742::sAddr_SMR))
            .WillOnce(Return(std::pair{false, 0}))
            .WillOnce(Return(std::pair{true, addr}));
        EXPECT_CALL(mEthMock, writeReg(addr, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
            .WillOnce(Return(true));
        EXPECT_CALL(mBaseMock, tick())
            .WillOnce(Return(0));
        EXPECT_CALL(mEthMock, readReg(addr, Lan8742::sAddr_BCR))
            .WillOnce(Return(std::pair{true, 0}));
        EXPECT_CALL(mBaseMock, wait(Lan8742::sInitializationWaitTime_ms))
            .Times(1);
    }
};

TEST_F(Lan8742Test, CannotFindDevice) {
    EXPECT_CALL(mEthMock, readReg(_, Lan8742::sAddr_SMR))
        .WillRepeatedly(Return(std::pair{false, 0}));
    auto status = mPhy.init<BaseMockStatic, EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::AddressError));
}

TEST_F(Lan8742Test, CannotWriteToDevice) {
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_SMR))
        .WillOnce(Return(std::pair{true, 0}));
    EXPECT_CALL(mEthMock, writeReg(0, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
        .WillOnce(Return(false));
    auto status = mPhy.init<BaseMockStatic, EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::WriteError));
}

TEST_F(Lan8742Test, CannotReadBCRReg) {
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_SMR))
        .WillOnce(Return(std::pair{true, 0}));
    EXPECT_CALL(mEthMock, writeReg(0, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
        .WillOnce(Return(true));
    EXPECT_CALL(mBaseMock, tick())
        .Times(1);
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_BCR))
        .WillOnce(Return(std::pair{false, 0}));
    auto status = mPhy.init<BaseMockStatic, EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::ReadError));
}

TEST_F(Lan8742Test, TimeoutBeforeReset) {
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_SMR))
        .WillOnce(Return(std::pair{true, 0}));
    EXPECT_CALL(mEthMock, writeReg(0, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
        .WillOnce(Return(true));
    EXPECT_CALL(mBaseMock, tick())
        .WillOnce(Return(0))
        .WillRepeatedly(Return(2*Lan8742::sResetTimeout_ms));
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_BCR))
        .WillRepeatedly(Return(std::pair{true, 0xFFFFFFFF}));
    auto status = mPhy.init<BaseMockStatic, EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::ResetTimeout));
}

TEST_F(Lan8742Test, InitSequence) {
    initExpectationsWithDevAddr1();
    auto status = mPhy.init<BaseMockStatic, EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::Ok));
}

TEST_F(Lan8742Test, LinkDownTest) {
    initExpectationsWithDevAddr1();
    mPhy.init<BaseMockStatic, EthMockStatic>();

    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BSR))
        .WillOnce(Return(std::pair{true, 0}));
    auto status = mPhy.getLinkState<EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::LinkDown));
}

TEST_F(Lan8742Test, HalfDuplex100Mbit) {
    initExpectationsWithDevAddr1();
    mPhy.init<BaseMockStatic, EthMockStatic>();

    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BSR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BSR_LINK_STATUS}));
    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BCR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BCR_SPEED_SELECT}));
    auto status = mPhy.getLinkState<EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::HalfDuplex100Mbit));
}

TEST_F(Lan8742Test, AutoNegNotDone) {
    initExpectationsWithDevAddr1();
    mPhy.init<BaseMockStatic, EthMockStatic>();

    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BSR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BSR_LINK_STATUS}));
    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BCR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BCR_AUTONEGO_EN}));
    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_PHYSCSR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_PHYSCSR_10BT_FD}));
    auto status = mPhy.getLinkState<EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::AutonegoNotDone));
}

TEST_F(Lan8742Test, FullDuplex10MbitAutoNegEn) {
    initExpectationsWithDevAddr1();
    mPhy.init<BaseMockStatic, EthMockStatic>();

    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BSR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BSR_LINK_STATUS}));
    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_BCR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_BCR_AUTONEGO_EN}));
    EXPECT_CALL(mEthMock, readReg(1, Lan8742::sAddr_PHYSCSR))
        .WillOnce(Return(std::pair{true, Lan8742::sMask_PHYSCSR_10BT_FD | Lan8742::sMask_PHYSCSR_AUTONEGO_DONE}));
    auto status = mPhy.getLinkState<EthMockStatic>();
    ASSERT_THAT(status, Eq(Lan8742::Status::FullDuplex10Mbit));
}

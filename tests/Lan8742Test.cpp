#include "gmock/gmock.h"
#include "Lan8742.h"
#include "EthMock.h"
#include "BaseMock.h"

using namespace ::testing;

class Lan8742Test: public Test {
public:
    EthMock mEthMock;
    BaseMock mBaseMock;
    Lan8742 mPhy{mBaseMock, mEthMock};
};

TEST_F(Lan8742Test, CannotFindDevice) {
    EXPECT_CALL(mEthMock, readReg(_, Lan8742::sAddr_SMR))
        .WillRepeatedly(Return(std::pair{false, 0}));
    auto status = mPhy.init();
    ASSERT_THAT(status, Eq(Lan8742::Status::AddressError));
}

TEST_F(Lan8742Test, CannotWriteToDevice) {
    EXPECT_CALL(mEthMock, readReg(0, Lan8742::sAddr_SMR))
        .WillOnce(Return(std::pair{true, 0}));
    EXPECT_CALL(mEthMock, writeReg(0, Lan8742::sAddr_BCR, Lan8742::sMask_BCR_SOFTRESET))
        .WillOnce(Return(false));
    auto status = mPhy.init();
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
    auto status = mPhy.init();
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
    auto status = mPhy.init();
    ASSERT_THAT(status, Eq(Lan8742::Status::ResetTimeout));
}

TEST_F(Lan8742Test, InitSequence) {
    const uint32_t addr{1};
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
    auto status = mPhy.init();
    ASSERT_THAT(status, Eq(Lan8742::Status::Ok));
}

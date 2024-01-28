#pragma once

#include <cstdint>
#include <span>

namespace lwipserver::stm32h7 {

class TxDescriptor {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    static constexpr uint32_t sDesc2IOC = 0x80000000;
    static constexpr uint32_t sDesc3OWN = 0x80000000;
    static constexpr uint32_t sDesc3FD  = 0x20000000;
    static constexpr uint32_t sDesc3LD  = 0x10000000;
    static constexpr uint32_t sDesc3CIC = 0x00030000;
    
    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    enum class Order {
        FirstDescriptor,
        LastDescriptor,
        NeitherFirstOrLast
    };

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    void set(std::span<uint8_t> buf1, std::span<uint8_t> buf2, uint32_t payloadLength, Order order) {
        mDesc0 = (buf1.size() != 0) ? reinterpret_cast<uint32_t>(buf1.data()) : 0;
        mDesc1 = (buf2.size() != 0) ? reinterpret_cast<uint32_t>(buf2.data()) : 0;
        mDesc2 = (buf2.size() << 16) | buf1.size();
        mDesc3 = sDesc3CIC | payloadLength;
        uint32_t desc3 = sDesc3OWN | sDesc3CIC | payloadLength;
        if (order == Order::FirstDescriptor) {
            mDesc3 = desc3 | sDesc3FD;
        } else if (order == Order::LastDescriptor) {
            mDesc3 = desc3 | sDesc3LD;
        } else {
            mDesc3 = desc3;
        }
    }

    bool ownedByDMA(void) const {
        return (mDesc3 & sDesc3OWN) == sDesc3OWN;
    }

    void setAppData(uint32_t data0, uint32_t data1) {
        mAppData0 = data0;
        mAppData1 = data1;
    }

    uint32_t getAppData0(void) const {
        return mAppData0;
    }

    uint32_t getAppData1(void) const {
        return mAppData1;
    }

private:

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    volatile uint32_t mDesc0;
    volatile uint32_t mDesc1;
    volatile uint32_t mDesc2;
    volatile uint32_t mDesc3;
    uint32_t mAppData0;
    uint32_t mAppData1;

} __attribute__((packed));

static_assert(sizeof(TxDescriptor) == 24);

} // namespace lwipserver::stm32h7
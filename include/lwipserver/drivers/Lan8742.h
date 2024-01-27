#pragma once

#include <cstdint>

#include "lwipserver/concepts/Base.h"
#include "lwipserver/concepts/Eth.h"

namespace lwipserver::drivers {

class Lan8742 {
public:
    
    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    static constexpr uint32_t sInitializationWaitTime_ms{2000};
    static constexpr uint32_t sMaxDevAddr{31};
    static constexpr uint32_t sResetTimeout_ms{500};

    // Register addresses.
    static constexpr uint32_t sAddr_BCR{0x00U};
    static constexpr uint32_t sAddr_BSR{0x01U};
    static constexpr uint32_t sAddr_SMR{0x12U};
    static constexpr uint32_t sAddr_PHYSCSR{0x1FU};

    // Field masks.
    static constexpr uint32_t sMask_BCR_SOFTRESET{0x8000U};
    static constexpr uint32_t sMask_BCR_SPEED_SELECT{0x2000U};
    static constexpr uint32_t sMask_BCR_AUTONEGO_EN{0x1000U};
    static constexpr uint32_t sMask_BCR_DUPLEX_MODE{0x0100U};
    static constexpr uint32_t sMask_BSR_LINK_STATUS{0x0004U};
    static constexpr uint32_t sMask_SMR_PHYSADDR{0x1FU}; 
    static constexpr uint32_t sMask_PHYSCSR_AUTONEGO_DONE{0x1000U};
    static constexpr uint32_t sMask_PHYSCSR_HCDSPEEDMASK{0x001CU};
    static constexpr uint32_t sMask_PHYSCSR_10BT_HD{0x0004U};
    static constexpr uint32_t sMask_PHYSCSR_10BT_FD{0x0014U};
    static constexpr uint32_t sMask_PHYSCSR_100BTX_HD{0x0008U};
    static constexpr uint32_t sMask_PHYSCSR_100BTX_FD{0x0018U};

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// Return values for the functions in this module.
    enum class Status {
        Ok,
        ReadError,
        WriteError,
        AddressError,
        ResetTimeout,
        LinkDown,
        AutonegoNotDone,
        FullDuplex100Mbit,
        HalfDuplex100Mbit,
        FullDuplex10Mbit,
        HalfDuplex10Mbit
    };

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Initialize the registers on the LAN8742 device.
    ///
    /// @return
    ///     Indicates an issue with initialization.
    template <typename Base, typename Eth>
        requires concepts::Base<Base> && concepts::Eth<Eth>
    Status init() {

        // Get the device address from special mode register. Search through all supported addresses until the correct
        // one is found.
        for (uint32_t addr = 0; addr <= sMaxDevAddr; addr++) {
            const auto val = Eth::readReg(addr, sAddr_SMR);
            if (val.first && (val.second & sMask_SMR_PHYSADDR) == addr) {
                mDevAddr = addr;
                break;
            } else if (addr == sMaxDevAddr) {
                return Status::AddressError;
            }
        }

        // Reset the device with a software reset.
        if (!Eth::writeReg(mDevAddr, sAddr_BCR, sMask_BCR_SOFTRESET)) { 
            return Status::WriteError;
        }

        // Check software reset clears within a timeout.
        uint32_t timeoutStart = Base::tick();
        while (true) {
            const auto val = Eth::readReg(mDevAddr, sAddr_BCR);
            if (!val.first) {
                return Status::ReadError;
            }
            if ((val.second & sMask_BCR_SOFTRESET) == 0) {
                break;
            }
            if (Base::tick() - timeoutStart > sResetTimeout_ms) {
                return Status::ResetTimeout;
            }
        }

        // Wait for IC to perform initialization.
        Base::wait(sInitializationWaitTime_ms);
        return Status::Ok;
    }
     
    /// Queries the phy for the status of the link.
    ///
    /// @return
    ///     Status indicating speed, duplex mode, if the link is up or down, or if there was an error reading
    ///     from the phy.
    template <typename Eth>
        requires concepts::Eth<Eth>
    Status getLinkState() {

        const auto [ok1, bsr] = Eth::readReg(mDevAddr, sAddr_BSR);
        if (!ok1) {
            return Status::ReadError;
        } 
        if ((bsr & sMask_BSR_LINK_STATUS) == 0) {
            return Status::LinkDown;    
        }
        
        const auto [ok2, bcr] = Eth::readReg(mDevAddr, sAddr_BCR);
        if (!ok2) {
            return Status::ReadError;
        }
        if ((bcr & sMask_BCR_AUTONEGO_EN) != sMask_BCR_AUTONEGO_EN) {
            if (((bcr & sMask_BCR_SPEED_SELECT) == sMask_BCR_SPEED_SELECT) && 
                ((bcr & sMask_BCR_DUPLEX_MODE) == sMask_BCR_DUPLEX_MODE)) {
                return Status::FullDuplex100Mbit;
            }
            if ((bcr & sMask_BCR_SPEED_SELECT) == sMask_BCR_SPEED_SELECT) {
                return Status::HalfDuplex100Mbit;
            }        
            if ((bcr & sMask_BCR_DUPLEX_MODE) == sMask_BCR_DUPLEX_MODE) {
                return Status::FullDuplex10Mbit;
            }
            return Status::HalfDuplex10Mbit;	
        } else {
            const auto [ok3, physcsr] = Eth::readReg(mDevAddr, sAddr_PHYSCSR);
            if (!ok3) {
                return Status::ReadError;
            }
            if ((physcsr & sMask_PHYSCSR_AUTONEGO_DONE) == 0) {
                return Status::AutonegoNotDone;
            }
            if ((physcsr & sMask_PHYSCSR_HCDSPEEDMASK) == sMask_PHYSCSR_100BTX_FD) {
                return Status::FullDuplex100Mbit;
            }
            if ((physcsr & sMask_PHYSCSR_HCDSPEEDMASK) == sMask_PHYSCSR_100BTX_HD) {
                return Status::HalfDuplex100Mbit;
            }
            if ((physcsr & sMask_PHYSCSR_HCDSPEEDMASK) == sMask_PHYSCSR_10BT_FD) {
                return Status::FullDuplex10Mbit;
            }
            return Status::HalfDuplex10Mbit;		
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The ethernet phy device address.
    uint32_t mDevAddr;  

};

} // namespace lwipserver::drivers
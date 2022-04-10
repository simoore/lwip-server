#include <cstdint>
#include "Lan8742.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

Lan8742::Status Lan8742::init() {

    // Get the device address from special mode register. Search through all supported addresses until the correct
    // one is found.
    for (uint32_t addr = 0; addr <= sMaxDevAddr; addr++) {
        const auto val = mEth.readReg(addr, sAddr_SMR);
        if (val.first && (val.second & sMask_SMR_PHYSADDR) == addr) {
            mDevAddr = addr;
            break;
        } else if (addr == sMaxDevAddr) {
            return Status::AddressError;
        }
    }

    // Reset the device with a software reset.
    if (!mEth.writeReg(mDevAddr, sAddr_BCR, sMask_BCR_SOFTRESET)) { 
        return Status::WriteError;
    }

    // Check software reset clears within a timeout.
    uint32_t timeoutStart = mBase.tick();
    while (true) {
        const auto val = mEth.readReg(mDevAddr, sAddr_BCR);
        if (!val.first) {
            return Status::ReadError;
        }
        if ((val.second & sMask_BCR_SOFTRESET) == 0) {
            break;
        }
        if (mBase.tick() - timeoutStart > sResetTimeout_ms) {
            return Status::ResetTimeout;
        }
    }

    // Wait for IC to perform initialization.
    mBase.wait(sInitializationWaitTime_ms);
    return Status::Ok;
}


Lan8742::Status Lan8742::getLinkState() {

    const auto [ok1, bsr] = mEth.readReg(mDevAddr, sAddr_BSR);
    if (!ok1) {
        return Status::ReadError;
    } 
    if ((bsr & sMask_BSR_LINK_STATUS) == 0) {
        return Status::LinkDown;    
    }
    
    const auto [ok2, bcr] = mEth.readReg(mDevAddr, sAddr_BCR);
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
        const auto [ok3, physcsr] = mEth.readReg(mDevAddr, sAddr_BCR);
        if(!ok3) {
            return Status::ReadError;
        }
        if((physcsr & sMask_PHYSCSR_AUTONEGO_DONE) == 0) {
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

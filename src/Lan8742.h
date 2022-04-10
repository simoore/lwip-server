#ifndef LAN8742_H
#define LAN8742_H

#include <cstdint>
#include "base/IBase.h"
#include "base/IEth.h"

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

    Lan8742(IBase &base, IEth &eth): mBase(base), mEth{eth} {}

    /// Initialize the registers on the LAN8742 device.
    ///
    /// @return
    ///     Indicates an issue with initialization.
    Status init();
     
    /// Queries the phy for the status of the link.
    ///
    /// @return
    ///     Status indicating speed, duplex mode, if the link is up or down, or if there was an error reading
    ///     from the phy.
    Status getLinkState();

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    uint32_t mDevAddr;  ///< The ethernet phy device address.
    IBase &mBase;       ///< Contains common functions provided by the embedded device.
    IEth &mEth;         ///< A reference to a ethernet peripheral that is using this PHY.

}; // class Lan8742

#endif // LAN8742_H
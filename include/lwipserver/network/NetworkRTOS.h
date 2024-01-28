#pragma once

#include <cstdint>

#include "lwip/init.h"
#include "lwip/timeouts.h"

#include "lwipserver/concepts/Base.h"
#include "lwipserver/freertos/OsTask.h"
#include "lwipserver/network/EthernetifCpp.h"
#include "lwipserver/utils/LoopTimer.h"

// Sort out EthernetifCpp interface for the RTOS version
// Keep link check thread in this file

namespace lwipserver::network {

/// This class encaptulates initialization and servicing of LwIP TCP/IP stack and the ethernet interface.
class Network final {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// The period at which the ethernet link state is checked.
    static constexpr uint32_t sLinkTimerPeriod_ms{100};

    /// The period at which the DHCP process function is called.
    static constexpr uint32_t sDhcpTimerPeriod_ms{500};

    /// The maximum number of tries to attain a IP address via DHCP.
    static constexpr uint32_t sMaxDhcpTries{2};

    /// Compile time flag inidicating use of DHCP.
#ifdef LWIP_DHCP
    static constexpr bool sUsingDHCP{true};
#else
    static constexpr bool sUsingDHCP{false};
#endif

    /// Compile time flag inidicating use of link callback in ethernet driver.
#if LWIP_NETIF_LINK_CALLBACK
    static constexpr bool sUsingLinkCallback{true};
#else
    static constexpr bool sUsingLinkCallback{false};
#endif

    /// The size of the DCHP Task in words.
    static constexpr uint32_t sLwIPTaskStackSize = 8 * (configMINIMAL_STACK_SIZE);

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// States used to monitor the network managers handling of the DHCP clients attempt to attain an IP address.
    enum class DhcpState {
        Off,                ///< Nothing is happening.
        Start,              ///< When the link is connected, this flags the start of attaining an IP address.
        WaitAddress,        ///< State while waiting for an address to be assigned.
        AddressAssigned,    ///< An Address has been successfully assigned.
        Timeout,            ///< Timeout after too many attempts to attain an IP address.
        LinkDown            ///< Flags the cable is disconnected and ceases any attempts to attain an address.
    };

    /// Callback from interface that indicates the status of the link has changed. It is used to start/stop the DHCP
    /// process.
    using LinkCallback = EthernetifCpp::LinkCallback;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// This initializes the components of the network stack. It doesn't initialize the applications that use it such 
    /// as the echo server. It initializes the LwIP stack and the ethernet interface. The LwIP stack must be 
    /// initialized first.
    void init(void) {

        lwip_init();

        // TODO: pass in callback in init.
        mInterface.registerLinkCallback([this] { linkStatusUpdated(); });
        mInterface.init();

        mLinkTimer.setExpiry(sLinkTimerPeriod_ms);
        mDHCPTimer.setExpiry(sDhcpTimerPeriod_ms);
        mLinkTimer.registerCallback(std::bind(&EthernetifCpp::checkLinkState, &mInterface));
        mDHCPTimer.registerCallback(std::bind(&Network::dhcpProcess, this));
    }

    /// This function is called when the state fo the ethenet link changes. The LwIP option LWIP_NETIF_LINK_CALLBACK
    /// must be set to use it. We publish the state of the link using printf and we notify the DHCP process if the 
    /// link is up or down.
    void linkStatusUpdated(void) {
        if (mInterface.isLinkUp()) {
            printf("EthernetInterface::linkStatusUpdate, link is up.\n");
            mDhcpState = DhcpState::Start;
        } else {
            printf("EthernetInterface::linkStatusUpdate, link is down.\n");
            mDhcpState = DhcpState::LinkDown;
        }
    }

    /// We monitor the status of the DHCP client to determine if we should attempt to attain an IP address using
    /// DHCP when the ethernet link is up, or if the DHCP has timed-out and we should apply a static IP.
    void dhcpProcess(void) {
        switch (mDhcpState) {
        case DhcpState::Start:
            printf("NetworkManager::dhcpProcess, looking for DHCP server.\n");
            mInterface.dhcpStart();
            mDhcpState = DhcpState::WaitAddress;
            break;
        case DhcpState::WaitAddress:
            if (mInterface.isDhcpSuppliedAddress()) {
                mDhcpState = DhcpState::AddressAssigned;
                printf("NetworkManager::dhcpProcess, DHCP assigned address: %s\n", mInterface.ipAddrStr());
            } else if (mInterface.dhcpTries() > sMaxDhcpTries) {
                // DHCP timeout - use static IP address.
                mDhcpState = DhcpState::Timeout;
                mInterface.setStaticIp();
                printf("NetworkManager::dhcpProcess, DHCP timeout, using static IP %s\n", mInterface.ipAddrStr());
            }
            break;
        case DhcpState::LinkDown:
            mDhcpState = DhcpState::Off;
            printf("The network cable is not connected.\n");
            break;
        default:
            break;
        }
    }

    template <typename Base>
        requires concepts::Base<Base>
    void lwipThread(void) {
        mInterface.service();
        sys_check_timeouts();

        // Check status of link periodically.
        mLinkTimer.poll<Base>();
        mDHCPTimer.poll<Base>();
    }

private: 

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    /// The ethernet interface handler.
    EthernetifCpp mInterface;  

    /// A state machine monitors the status of the DHCP client.
    DhcpState mDhcpState{DhcpState::LinkDown};

    /// Executes the DHCP process to attain an IP address.
    utils::LoopTimer mDHCPTimer;

    /// Executes a task to check the status of the ethernet link.
    utils::LoopTimer mLinkTimer;

};

} // namespace lwipserver::network

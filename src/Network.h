#ifndef NETWORK_H
#define NETWORK_H

// #include "lwip/opt.h"
// #include "lwip/init.h"
// #include "netif/etharp.h"
// #include "lwip/netif.h"
// #include "lwip/timeouts.h"
// #include "lwip/dhcp.h"
// #include "ethernetif.h"
// #include "app_ethernet.h"
// #include "tcp_echoserver.h"

#include "EthernetInterface.h"

/// This class encaptulates initialization and servicing of LwIP TCP/IP stack and the ethernet interface.
class Network {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// The period at which the ethernet link state is checked.
    static constexpr uint32_t sLinkTimerPeriod_ms{100};

    /// The period at which the DHCP process function is called.
    static constexpr uint32_t sDhcpTimerPeriod_ms{500};

    /// The maximum number of tries to attain a IP address via DHCP.
    static constexpr uint32_t sMaxDhcpTries{4};

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// Configuration of the network. Includes the STM32 HAL configuration of the ethernet peripheral.
    struct Config {
        EthernetInterface::Config ethernetInterfaceCfg; ///< Ethernet peripheral config.
    };

    /// States used to monitor the network managers handling of the DHCP clients attempt to attain an IP address.
    enum class DhcpState {
        Off,                ///< Nothing is happening.
        Start,              ///< When the link is connected, this flags the start of attaining an IP address.
        WaitAddress,        ///< State while waiting for an address to be assigned.
        AddressAssigned,    ///< An Address has been successfully assigned.
        Timeout,            ///< Timeout after too many attempts to attain an IP address.
        LinkDown            ///< Flags the cable is disconnected and ceases any attempts to attain an address.
    };

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// This initializes the components of the network stack. It doesn't initialize the applications that use it such 
    /// as the echo server. It initializes the LwIP stack and the ethernet interface. The LwIP stack must be 
    /// initialized first.
    ///
    /// @param cfg
    ///     The configuration of the network stack.
    void init(const Config &cfg);

    /// This is the main loop function that services the LwIP stack and the ethernet interface.
    ///
    /// When LwIP option NO_SYS=1, you must call the sys_check_timeouts() function in the main loop for the stack
    /// to periodically check if and timers in LwIP have expired.
    void service();

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// This function is called when the state fo the ethenet link changes. The LwIP option LWIP_NETIF_LINK_CALLBACK
    /// must be set to use it. We publish the state of the link using printf and we notify the DHCP process if the 
    /// link is up or down.
    void linkStatusUpdated();

    /// We monitor the status of the DHCP client to determine if we should attempt to attain an IP address using
    /// DHCP when the ethernet link is up, or if the DHCP has timed-out and we should apply a static IP.
    void dhcpProcess();

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The ethernet interface handler.
    EthernetInterface mInterface; 

    uint32_t mLinkTimer{0};     ///< Millisecond counts since we last checked the link state.
    uint32_t mDhcpTimer{0};     ///< Millisecond counts since we last executed the DHCP process.

    /// A state machine monitors the status of the DHCP client.
    DhcpState mDhcpState{DhcpState::LinkDown};

}; // class NetworkManager

#endif // NETWORK_MANAGER_H
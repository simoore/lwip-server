#ifndef NETWORK_H
#define NETWORK_H

#include <cstdint>
#include "eth_freertos.h"
#include "base/IBase.h"
#include "FreeRTOS.h"

// Sort out EthernetifCpp interface for the RTOS version
// Keep link check thread in this file

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
    static constexpr uint32_t sDhcpTaskStackSize = 2 * (configMINIMAL_STACK_SIZE);

    /// The size of the Check Link Task in words.
    static constexpr uint32_t sCheckLinkTaskStackSize = 2 * (configMINIMAL_STACK_SIZE);

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

    Network(IBase &base): mBase(base) {}

    /// This initializes the components of the network stack. It doesn't initialize the applications that use it such 
    /// as the echo server. It initializes the LwIP stack and the ethernet interface. The LwIP stack must be 
    /// initialized first.
    ///
    /// @param cfg
    ///     The configuration of the network stack.
    void init();

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

    /// This is the thread launched to check the network interface link status.
    ///
    /// @param args
    ///     This is the pointer to the network stack.
    static void checkLinkThread(void *args);

    /// This is the thread launched to service DHCP process when using an RTOS.
    ///
    /// @param args
    ///     This is the pointer to the network stack.
    static void dhcpThread(void *args);

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    /// Provides base platform fuctions.
    IBase &mBase;

    /// The ethernet interface handler.
    EthernetifCpp mInterface;  

    /// A state machine monitors the status of the DHCP client.
    DhcpState mDhcpState{DhcpState::LinkDown};

}; // class Network

#endif // NETWORK_H
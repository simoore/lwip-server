#pragma once

#include <functional>

#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);
void ethernet_link_check_state(struct netif *netif);

namespace lwipserver::stm32h7 {

/// This is a C++ wrapper class for ST's ethernet driver for STM32H7.
class Ethernetif final {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    static constexpr uint8_t IP_ADDR0{192U};
    static constexpr uint8_t IP_ADDR1{168U};
    static constexpr uint8_t IP_ADDR2{112U};
    static constexpr uint8_t IP_ADDR3{10U};

    static constexpr uint8_t NETMASK_ADDR0{255U};
    static constexpr uint8_t NETMASK_ADDR1{255U};
    static constexpr uint8_t NETMASK_ADDR2{255U};
    static constexpr uint8_t NETMASK_ADDR3{0U};

    static constexpr uint8_t GW_ADDR0{192U};
    static constexpr uint8_t GW_ADDR1{168U};
    static constexpr uint8_t GW_ADDR2{112U};
    static constexpr uint8_t GW_ADDR3{1U};

#if LWIP_DHCP
    static constexpr bool sUseDHCP = true;
#else
    static constexpr bool sUseDHCP = false;
#endif

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// The network interface handle struct from LwIP.
    using Netif = struct netif;

    /// The DHCP handle struct from LwIP.
    using Dhcp = struct dhcp;

    /// Callback to application for when the link status changes.
    using LinkCallback = std::function<void(void)>;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    void init() {
        ip_addr_t ipaddr;
        ip_addr_t netmask;
        ip_addr_t gw;

        if constexpr (sUseDHCP) {
            ip_addr_set_zero_ip4(&ipaddr);
            ip_addr_set_zero_ip4(&netmask);
            ip_addr_set_zero_ip4(&gw);
        } else {
            IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
            IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
            IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
        }

        netif_add(&gnetif, &ipaddr, &netmask, &gw, this, &ethernetif_init, &ethernet_input);
        netif_set_default(&gnetif);
        if (mLinkCallback) {
            mLinkCallback();
        }

#if LWIP_NETIF_LINK_CALLBACK
        netif_set_link_callback(&gnetif, lwipLinkCallback);
#endif
    }

    /// Register a callback that gets executed when the link state changes.
    ///
    /// @param cb
    ///     The callback.
    void registerLinkCallback(LinkCallback cb) {
        mLinkCallback = cb;
    }

    /// Ask the ETH peripheral to ask the PHY what the link state is.
    void checkLinkState() {
        ethernet_link_check_state(&gnetif);
    }

    /// The ethernet peripheral needs to check for received data periodically.
    void service() {
        ethernetif_input(&gnetif);
    }

    /// Asks the network adapter if the link is up. This is set when the link state is checked.
    bool isLinkUp() const {
        return netif_is_link_up(&gnetif);
    }

    /// Starts the DHCP process to aquire an IP address.
    void dhcpStart() {
        if constexpr (sUseDHCP) {
            ip_addr_set_zero_ip4(&gnetif.ip_addr);
            ip_addr_set_zero_ip4(&gnetif.netmask);
            ip_addr_set_zero_ip4(&gnetif.gw);
            dhcp_start(&gnetif);
        }
    }

    bool isDhcpSuppliedAddress() const {
        if constexpr (sUseDHCP) {
            return dhcp_supplied_address(&gnetif);
        } else {
            return false;
        }
    }

    uint32_t dhcpTries() const {
        if constexpr (sUseDHCP) {
            Dhcp *dhcp = reinterpret_cast<Dhcp *>(netif_get_client_data(&gnetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP));
            return static_cast<uint32_t>(dhcp->tries);
        } else {
            return 0;
        }
    }

    void setStaticIp() {
        ip_addr_t ipaddr;
        ip_addr_t netmask;
        ip_addr_t gw;
        IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
        IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
        IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
        netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
    }

    /// Returns string representation of IP address of the network interface.
    ///
    /// @return
    ///     The IP address of this network interface.
    const char *ipAddrStr() {
        return ip4addr_ntoa(netif_ip4_addr(&gnetif));
    }

private:

    /// LwIP callback for when the link is brought up or down.
    ///
    /// @param netif
    ///     Ignored netif for the link whose status has changed.
    static void lwipLinkCallback(Netif *netif) {
        auto *interface = reinterpret_cast<Ethernetif *>(netif->state);
        if (interface->mLinkCallback) {
            interface->mLinkCallback();
        }
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    /// LwIP network interface handle.
    Netif gnetif;

    /// LwIP executes this callback when the link goes up or down.
    LinkCallback mLinkCallback;

};

} // namespace lwipserver::network
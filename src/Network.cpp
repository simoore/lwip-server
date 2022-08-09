#include "etl/delegate.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "Network.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

void Network::init(const Config &cfg) {
    lwip_init();
    mInterface.init(cfg.ethernetInterfaceCfg);
    mInterface.registerLinkCallback(etl::delegate<void(void)>::create<NetworkManager, 
        &NetworkManager::linkStatusUpdated>(*this));
}

void Network::service() {
    mInterface.service();
    sys_check_timeouts();
    
    // Check status of link periodically.
    uint32_t now = HAL_GetTick();
    #if LWIP_NETIF_LINK_CALLBACK
    if (now - mLinkTimer >= sLinkTimerPeriod_ms) {
        mLinkTimer = now;
        mInterface.checkLinkState();
    }
    #endif

    // Execute DHCP process periodically.
    #if LWIP_DHCP
    if (now - mDhcpTimer >= sDhcpTimerPeriod_ms) {
        mDhcpTimer = now;
        dhcpProcess();
    }
    #endif
}

/*************************************************************************/
/********** PRIVATE FUNCTIONS ********************************************/
/*************************************************************************/

void Network::linkStatusUpdated() {
    if (mInterface.isLinkUp()) {
        printf("EthernetInterface::linkStatusUpdate, link is up.\n");
        mDhcpState = DhcpState::Start;
    } else {
        printf("EthernetInterface::linkStatusUpdate, link is down.\n");
        mDhcpState = DhcpState::LinkDown;
    }
}

void Network::dhcpProcess() {
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

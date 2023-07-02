#include "Network.h"
#include "etl/delegate.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "task.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

void Network::init() {
    lwip_init();
    // TODO: pass in callback in init.
    mInterface.registerLinkCallback(LinkCallback::create<Network, &Network::linkStatusUpdated>(*this));
    mInterface.init();

    if constexpr (sUsingRTOS) {
        if constexpr (sUsingLinkCallback) {
            auto status = xTaskCreate(Network::checkLinkThread, "CheckLinkTask", sCheckLinkTaskStackSize, this, 
                PRIORITY_NORMAL, nullptr);
            if (status == pdFAIL) {
                while (true);
            }
        }

        if constexpr (sUsingDHCP) {
            auto status = xTaskCreate(Network::dhcpThread, "DHCPTask", sDhcpTaskStackSize, this, 
                PRIORITY_BELOW_NORMAL, nullptr);
            if (status == pdFAIL) {
                while (true);
            }
        }
    }
}

void Network::service() {
    mInterface.service();
    sys_check_timeouts();

    // Check status of link periodically.
    uint32_t now = mBase.tick();
    if constexpr (sUsingLinkCallback) {
        if ((now - mLinkTimer) >= sLinkTimerPeriod_ms) {
            mLinkTimer = now;
            mInterface.checkLinkState();
        }
    }

    // Execute DHCP process periodically.
    if constexpr (sUsingDHCP) {
        if ((now - mDhcpTimer) >= sDhcpTimerPeriod_ms) {
            mDhcpTimer = now;
            dhcpProcess();
        }
    }
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

/*************************************************************************/
/********** RTOS THREADS *************************************************/
/*************************************************************************/

void Network::checkLinkThread(void *args) {
    Network &network = *reinterpret_cast<Network *>(args);
    while (true) {
        network.dhcpProcess();
        vTaskDelay(pdMS_TO_TICKS(sDhcpTimerPeriod_ms));
    }

    // If for some reason the task breaks out of its loop, it must be deleted. The nullptr indicates that this
    // calling task is the one to be deleted.
    vTaskDelete(nullptr);
}

void Network::dhcpThread(void *args) {
    Network &network = *reinterpret_cast<Network *>(args);
    while (true) {
        network.mInterface.checkLinkState();
        vTaskDelay(pdMS_TO_TICKS(sLinkTimerPeriod_ms));
    }
    vTaskDelete(nullptr);
}
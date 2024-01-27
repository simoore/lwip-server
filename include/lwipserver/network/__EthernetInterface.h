#ifndef ETHERNET_INTERFACE_H
#define ETHERNET_INTERFACE_H

#include <cstring>
#include <algorithm>
#include "etl/delegate.h"
#include "etl/queue.h"
#include "etl/queue_spsc_atomic.h"
#include "etl/vector.h"
#include "lan8742.h"
#include "lwip/dhcp.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"


/// This class encapulates the ethernet driver as required by LwIP.
class EthernetInterface {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// Size of an ethernet RX buffer. Generally you want this size to be greater than the typical size of the 
    /// application protocols packet. A size of 1536 is common because...
    static constexpr uint32_t sRxBufferSize{1024};

    /// The number of RX buffers. This is more than the number of descriptors because a buffer can be utilized by LwIP 
    /// or the application, and the descriptor is assigned to another buffer.
    static constexpr uint32_t sRxBufferCount{6};

    /// Used for the number of descriptors that are created per transmitted frame. This make this number the maximum
    /// number of pbufs in the chain for a transmitted frame.
    static constexpr uint32_t sTxBufferCount{6};

    static_assert(sRxBufferSize % 32 == 0, "RX Buffer size must be multiple of 32 bytes.");
    //static_assert(sRxBufferCount >= sEthRxDescCnt, "Must have more buffers than descriptors.");
    //static_assert(sTxBufferCount >= sEthTxDescCnt, "Must have more buffers than descriptors.");

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// This LwIP network interface handle.
    using NetworkInterface = struct netif;

    /// Packet buffers are LwIP containers that contain the network packets.
	using PacketBuffer = struct pbuf;

    /// LwIP packet buffer with custom de-allocator function.
    using PacketBufferCustom = struct pbuf_custom;

    /// LwIP handle for DHCP.
    using Dhcp = struct dhcp;

    using MempDesc = struct memp_desc;

    /// The RX buffer is composed of the actual buffer and a pbuf structure to manage it in LwIP.
    /// The buffer is aligned to a 32 byte boundary because some sort of caching.
    struct RxBuf {
        PacketBufferCustom pbufCustom;
        uint8_t buffer[sRxBufferSize] __ALIGNED(32);
    };

    /// Container of ethernet RX buffers.
    //using RxBuffers = RxBuf[sRxBufferCount];

    /// Container for packet buffers to be freed when transmission completes.
    using PacketBufferQueue = etl::queue<PacketBuffer *, sTxBufferCount>;

    /// Callback to application for when the link status changes.
    using LinkCallback = etl::delegate<void(void)>;

    using MacAddr = uint8_t[ETH_HWADDR_LEN];

    /// Ethernet interface configuration.
    struct Config {
        uint8_t ipAddr[4];                  ///< IP address if using static address.
        uint8_t networkMask[4];             ///< Network mask if using static address.
        uint8_t gateway[4];                 ///< Gateway address if using static address.
        uint8_t macAddr[ETH_HWADDR_LEN];    ///< The MAC address of this ethernet interface.
    };

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Constructor links the platform the ethernet and phy driver.
    EthernetInterface(IBase &base, IEth &eth): mEth(eth), mPhy(base, eth) {}

    /// Initialization sets IP address of network interface, registers the network interface with LwIP, and initializes
    /// the STM32 ethernet peripheral. 
    ///
    /// Reference: https://lwip.fandom.com/wiki/Network_interfaces_management
    ///
    /// @param cfg
    ///     Ethernet interface configuration.
    void init(const Config &cfg) {
        mConfig = cfg;

        ip_addr_t ipaddr, netmask, gateway;
        #if LWIP_DHCP
        ip_addr_set_zero_ip4(&ipaddr);
        ip_addr_set_zero_ip4(&netmask);
        ip_addr_set_zero_ip4(&gateway);
        #else
        IP4_ADDR(&ipaddr, cfg.ipAddr[0], cfg.ipAddr[1], cfg.ipAddr[2], cfg.ipAddr[3]);
        IP4_ADDR(&netmask, cfg.networkMask[0], cfg.networkMask[1] , cfg.networkMask[2], cfg.networkMask[3]);
        IP4_ADDR(&gateway, cfg.gateway[0], cfg.gateway[1], cfg.gateway[2], cfg.gateway[3]);
        #endif

        // ethernet_input() come from the LwIP. It process any ethernet packet, plus add ARP to this netif.
        netif_add(&mNetif, &ipaddr, &netmask, &gateway, NULL, &staticLwipInit, &ethernet_input);
        netif_set_default(&mNetif);

        #if LWIP_NETIF_LINK_CALLBACK
        // This sets the callback to be called when the link goes up/down.
        netif_set_link_callback(&mNetif, &staticLwipLinkCallback);
        #endif
    }

    /// Register a callback that gets executed when the link state changes.
    ///
    /// @param cb
    ///     The callback.
    void registerLinkCallback(LinkCallback cb) {
        mLinkCallback = cb;
    }

    /// From wikipedia, the LSB of the first octet is the I/G (Individual/Group) bit. When 0 the transmission is a 
    /// unicast, otherwise it is a multicast.
    ///
    /// @param firstOctet
    ///     The first octet of the MAC address. For outgoing ethernet packets, this is the first byte of the packet.
    /// @return
    ///     True if this MAC address is a unicast, otherwise it is a multicast.
    bool isUnicastMacAddr(uint8_t firstOctet) {
        return (firstOctet & 1) == 0;
    }

    /// Returns the IP address as a string. This pointer points to a static buffer with a null terminated string in it. 
    /// No need to copy.
    ///
    /// @return
    ///     The IP address string.
    const char *ipAddrStr() const {
        return ip4addr_ntoa(netif_ip4_addr(&mNetif));
    }

    bool isDhcpSuppliedAddress() const {
        return dhcp_supplied_address(&mNetif) == 1;
    }

    bool isLinkUp() const {
        return netif_is_link_up(&mNetif);
    }

    uint32_t dhcpTries() const {
        const auto *clientData = netif_get_client_data(&mNetif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
        const Dhcp *dhcp = reinterpret_cast<const Dhcp *>(clientData);
        return dhcp->tries;
    }

    void dhcpStart() {
        ip_addr_set_zero_ip4(&(mNetif.ip_addr));
        ip_addr_set_zero_ip4(&(mNetif.netmask));
        ip_addr_set_zero_ip4(&(mNetif.gw));
        dhcp_start(&mNetif);
    }

    void setStaticIp() {
        ip_addr_t ipaddr, netmask, gateway;
        IP4_ADDR(&ipaddr, mConfig.ipAddr[0], mConfig.ipAddr[1], mConfig.ipAddr[2], mConfig.ipAddr[3]);
        IP4_ADDR(&netmask, mConfig.networkMask[0], mConfig.networkMask[1] , mConfig.networkMask[2], 
            mConfig.networkMask[3]);
        IP4_ADDR(&gateway, mConfig.gateway[0], mConfig.gateway[1], mConfig.gateway[2], mConfig.gateway[3]);
        netif_set_addr(&mNetif, &ipaddr, &netmask, &gateway);
    }

    /// Read a received packet from the Ethernet buffers and send it to the LwIP for handling.
    void service() {    
        inputService();
        
        // If TX is suspended and there are still packets in the packet buffer, let's free them.
        if (mEth.status().isSuspended && !mTxPacketBuffers.empty()) {
            printf("EthernetInterface::service, freed TX PacketBuffers.");
            mTxPacketBuffers.clear();
        }
    }

    /// Query the phy to check the status of the link.
    void checkLinkState() {
        auto startEthernet = [&](IEth::DuplexMode duplexMode, IEth::Speed speed) {
            mEth.startEthernet(duplexMode, speed);
            netif_set_up(&mNetif);
            netif_set_link_up(&mNetif);
        };

        const auto linkState = mPhy.getLinkState();
        const bool stopEth = (linkState == Lan8742::Status::LinkDown) || (linkState == Lan8742::Status::ReadError);

        if (netif_is_link_up(&mNetif) && stopEth) {
            mEth.stopEthernet();
            netif_set_down(&mNetif);
            netif_set_link_down(&mNetif);
        } else if (!netif_is_link_up(&mNetif) && !stopEth) {
            switch (linkState) {
            case Lan8742::Status::FullDuplex100Mbit:
                startEthernet(IEth::DuplexMode::FullDuplex, IEth::Speed::Hundred);
                break;
            case Lan8742::Status::HalfDuplex100Mbit:
                startEthernet(IEth::DuplexMode::HalfDuplex, IEth::Speed::Hundred);
                break;
            case Lan8742::Status::FullDuplex10Mbit:
                startEthernet(IEth::DuplexMode::FullDuplex, IEth::Speed::Ten);
                break;
            case Lan8742::Status::HalfDuplex10Mbit:
                startEthernet(IEth::DuplexMode::HalfDuplex, IEth::Speed::Ten);
                break;
            default:
                break;      
            }
        }
    }

    /// LwIP passes a packet to this ethernet driver for transmission using this function.
    ///
    /// @param p
    ///     The packet buffer containing the ethernet packet to send.
    /// @return
    ///     ERR_OK if the packet was sent. Returns ERR_IF if there wasn't enough descriptors available.
    err_t output(PacketBuffer *p) {

        // We maintain a list of TX packet buffers so they can be freed after transmission has finished.
        if (mTxPacketBuffers.full()) {
            printf("EthernetInterface::output, packet buffer queue full.");
            return ERR_IF;
        }

        // We can pad the start of the ethernet packet to align the IP address on a 32bit boundary to increase the 
        // speed of processing the packet.
        #if ETH_PAD_SIZE
        pbuf_remove_header(p, ETH_PAD_SIZE);
        #endif

        // Create a set of generic descriptors from the LwIP packet chain.
        IEth::Packet packet;
        for (PacketBuffer *q = p; q != nullptr; q = q->next) {
            if (packet.descriptorList.full()) {
                printf("EthernetInterface::output, not enough descriptors.");
                return ERR_IF;
            }
            packet.descriptorList.push_back(IEth::Descriptor{ 
                .buffer = reinterpret_cast<uint8_t *>(q->payload), 
                .size = q->len 
            });
        }

        // Pass the pbuf to the driver and it will try to assign a descriptor to each buffer in the packet. It will
        // keep track of and free the pbuf when the packet is sent.
        const bool ok = mEth.writeData(packet);
        if (!ok) {
            printf("EthernetInterface::output, writeData not ok.")
            MIB2_STATS_NETIF_INC(&mNetif, ifoutdiscards);
            return ERR_IF;
        }
        mTxPacketBuffers.push(p);

        // If using stats, this will add p->tot_len to the counter ifoutoctets which keeps track of the number of 
        // octets trasmitted.
        MIB2_STATS_NETIF_ADD(&mNetif, ifoutoctets, p->tot_len);
        if (isUnicastMacAddr(*reinterpret_cast<u8_t *>(p->payload))) {
            MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
        } else {
            MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
        }
        
        #if ETH_PAD_SIZE
        pbuf_add_header(p, ETH_PAD_SIZE);
        #endif

        LINK_STATS_INC(link.xmit);
        return ERR_OK;
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// We attempt to allocate a buffer from the RX memory pool when the ETH peripheral asks for a buffer.
    /// https://lwip.fandom.com/wiki/Custom_memory_pools
    /// https://www.nongnu.org/lwip/2_1_x/zerocopyrx.html
    ///
    /// @return
    ///     Pointer to the allocated buffer, can be nullptr if allocation failed.
    uint8_t *rxAllocateBuffer() {
        PacketBufferCustom *p = LWIP_MEMPOOL_ALLOC(sRxPool);
        if (p) {
            p->custom_free_function = &staticPbufFreeCustom;
            uint8_t *buffer = reinterpret_cast<uint8_t *>(p) + offsetof(RxBuf, buffer); 
            pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, buffer, sRxBufferSize);
            return buffer;
        } else {
            mRxAllocationError = true;
            printf("EthernetInterface::rxAllocateBuffer, failed to allocate RX buffer.\n");
            return nullptr;
        }
    }

    /// When the ETH peripheral executes the callback indicating it sent a packet, free the pbuf at the front of the
    /// queue.
    void txComplete() {
        if (mTxPacketBuffers.empty()) {
            printf("EthernetInterface::txComplate, TX packet queue empty.\n");
            return;
        }
        pbuf_free(mTxPacketBuffers.front());
        mTxPacketBuffers.pop();
    }

    /// This is called periodically in the main loop to check the network interface for a packet, and if one is
    /// available, pass it to the LwIP stack for processing. If an error occurs when processing the packet to LwIP, 
    /// we log the result and move on.
    void inputService() {
        if (mRxAllocationError) {
            return;
        }
        IEth::Packet *packet = mEth.readData();
        if (!packet) {
            return;
        }
        for (const auto &d : packet->descriptorList) {
            static_cast<void>(d);
            // create pbuf for LwIP
        }
        // TODO: replace nullptr by pbuf
        err_t err = mNetif.input(nullptr, &mNetif);
        if (err != ERR_OK) {
            LWIP_DEBUGF(NETIF_DEBUG, ("EthernetInterface::inputService, IP input error\n"));
            pbuf_free(nullptr);
        }
    }

    /// This is the driver specific initialization function called once the netif structure has been prepared by LwIP.
    ///
    /// Reference
    /// ---------
    /// https://lwip.fandom.com/wiki/Writing_a_device_driver
    err_t lwipInit() {

        #if LWIP_NETIF_HOSTNAME
        mNetif.hostname = "lwip";
        #endif
        mNetif.name[0] = 'e';
        mNetif.name[1] = 'n';
        
        #if LWIP_IPV4
        mNetif.output = &etharp_output;
        #endif
        #if LWIP_IPV6
        mNetif.output_ip6 = &ethip6_output;
        #endif
        mNetif.linkoutput = &staticOutput;
        mNetif.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
        //mNetif.mtu = ETH_MAX_PAYLOAD; // TODO: get from IEth

    //     /*
    //     * Initialize the snmp variables and counters inside the struct netif.
    //     * The last argument should be replaced with your link speed, in units
    //     * of bits per second.
    //     */ 
    //     // MIB2_INIT_NETIF
    //     // https://lwip-users.nongnu.narkive.com/VdZD2JXc/snmp
    //     //NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

        // Set MAC hardware address.
        mNetif.hwaddr_len = ETH_HWADDR_LEN;
        std::copy_n(std::begin(mMacAddress), ETH_HWADDR_LEN, std::begin(mNetif.hwaddr));



    //     // Set common parameters of TX packet config.
    //     memset(&mTxConfig, 0 , sizeof(ETH_TxPacketConfig));
    //     mTxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
    //     mTxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
    //     mTxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

        // Initialize the peripheral first, then add it to LwIP.
        //mEth.init(cfg.ethCfg); // These might already be initialized by this point.
        mPhy.init();
        checkLinkState();

        return ERR_OK;
    }



    /*************************************************************************/
    /********** PRIVATE FUNCTIONS - STATIC CALLBACKS *************************/
    /*************************************************************************/

    /// LwIP callback for when the link is brought up or down. Currently we assume there is only one network
    /// interface in the system so we ignore the netif parameter, and just retrieve the EthernetInterface instance
    /// and inform it the status of the link has changed.
    ///
    /// @param netif
    ///     Ignored netif for the link whose status has changed.
    static void staticLwipLinkCallback(NetworkInterface *netif) {
        // static_cast<void>(netif);
        // if (getInstance().mLinkCallback.is_valid()) {
        //     getInstance().mLinkCallback();
        // }
    }

    /// LwIP callback when it wants to initialize the device.
    ///
    /// @param netif
    ///     The network device that is being initialized by LwIP.
    /// @return
    ///     ERR_OK, initialization should always succeed.
    static err_t staticLwipInit(NetworkInterface *netif) {
        return reinterpret_cast<EthernetInterface *>(netif->state)->lwipInit();
    }

    /// LwIP callback to send data to this ethernet interface for transmission.
    ///
    /// @param netif
    ///     The LwIP network interface handle.
    /// @param p
    ///     The data to transmit.
    /// @return
    ///     ERR_IF if there isn't enough descriptors available to transmit the data, else ERR_OK.
    static err_t staticOutput(struct netif *netif, struct pbuf *p) {
        return reinterpret_cast<EthernetInterface *>(netif->state)->output(p);
    }


    /// De-allocated
    static void pbufCustomFree(PacketBuffer *p) {
        LWIP_MEMPOOL_FREE(sRxPool, reinterpret_cast<PacketBufferCustom *>(p));
        mRxAllocationError = false;
    }

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    IEth &mEth;                         ///< The abstracted hardware ethernet peripheral.
    Lan8742 mPhy;                       ///< The ethernet phy driver.
    NetworkInterface mNetif;            ///< LwIP network handle.
    bool mRxAllocationError{false};     ///< True if the last request to allocate failed.



    /// This queue stores the packet buffers that are currently being transmitted so they can be freed when 
    /// transmission is complete. You can have a most 1 packet buffer per TX buffer.
    PacketBufferQueue mTxPacketBuffers;

    Config mConfig;
    MacAddr mMacAddress{0, 0, 0, 0, 0, 0};
    LinkCallback mLinkCallback;
    

//     MempDesc mMempDesc;


}; // class EthernetInterface

#endif // ETHERNET_INTERFACE_H
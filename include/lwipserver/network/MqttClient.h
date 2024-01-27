#pragma once

#include "etl/delegate.h"
#include "etl/flat_map.h"
#include "lwip/apps/mqtt.h"

#include "lwipserver/concepts/Base.h"
#include "lwipserver/utils/LoopTimer.h"

/// Implements a MQTT client. To use:
/// 1. Call init(..) and check the return value to see if was successful.
/// 2. Call service(..) periodically in the application to run the module. 
/// 3. Subscribe to topics with registerSubscription(..). Messages will be dispatched to the application via a 
///    callback you set in the subscription.
/// 4. Publish data using publish(..). The callback set in the publication struct indicates if publication was 
///    successful.
class MqttClient {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    static constexpr size_t sMaxSubscriptions = 4;
    static constexpr uint8_t sQos = 0;
    static constexpr uint8_t sRemain = 0;
    static constexpr uint32_t sHealthCheckExpiry = 2000;
    static constexpr uint16_t sKeepAlive = 25;
    static constexpr uint8_t sDefaultIp[4] = {192U, 168U, 112U, 11U};
    static constexpr const char *sDefaultId = "LwIPServer";

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    struct Config {
        ip_addr_t brokerIpAddr;
        uint16_t brokerPort{MQTT_PORT};
        uint16_t keepAlive{sKeepAlive};

        /// The client ID is what is used to identify the client on the network. It should be unique amoungst all 
        /// clients connected to a broker.
        const char *clientId{sDefaultId};

        Config() {
            IP4_ADDR(&brokerIpAddr, sDefaultIp[0], sDefaultIp[1], sDefaultIp[2], sDefaultIp[3]);
        }
    };

    struct Publication {
        const char *topicName;
        const void *payload;
        uint16_t payloadSize;

        /// Flags whether the publication was successful or not.
        etl::delegate<void(bool)> publicationRequest;
    };

    struct Subscription {
        const char *topic;          /// The topic of this subscription.
        uint8_t *buffer;            /// The last received payload for this subscription.
        uint32_t capacity;          /// The size of buffer.
        uint32_t size;              /// The size of the payload.
        uint32_t totalSize;         /// The expected size of the payload read from the header.
        bool subscribed;            /// Whether subscribed to the broker.

        /// Callback to execute when entire payload is received.
        etl::delegate<void(void)> receivedPayload;
    };

    using Client = mqtt_client_t;
    using ClientInfo = struct mqtt_connect_client_info_t;
    
    /// Allows sorting by a subscriptions topic.
    class SubscriptionKey {
    public:
        
        SubscriptionKey(const char *topic) : mTopic(topic) {}

        bool operator<(const SubscriptionKey &rhs) const {
            if (mTopic == rhs.mTopic) {
                return false;
            }
            if (!mTopic) {
                return true;
            }
            if (!rhs.mTopic) {
                return false;
            }
            return strcmp(mTopic, rhs.mTopic) < 0;
        }

    private:
        const char *mTopic;
    };

    using SubscriptionMap = etl::flat_map<SubscriptionKey, Subscription *, sMaxSubscriptions>;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS: INTERFACE **********************************/
    /*************************************************************************/

    /// When connected to a broker, you can subscribe to and publish information via this client.
    bool connected() const {
        return mqtt_client_is_connected(mLwIPClient);
    }

    /// Allocates memory for the LwIP MQTT client and initializes the health check timer.
    bool init(const Config &cfg);

    /// Registers a subscription with the client so we know where to dispatch received messages to.
    bool registerSubscription(Subscription &subscription);

    /// Sends a publication to the broker.
    void publish(Publication &publication);

    /// Service periodically checks if the client is disconnected and attempts re-connection.
    template <typename Base>
        requires lwipserver::concepts::Base<Base>
    void service() {
        if (mLwIPClient) {
            mHealthCheckTimer.poll<Base>();
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Attempts to connect to the MQTT broker.
    void connect();

    /// Add a subscription to this client. Subscriptions will attempt to resubscribe periodically if they fail to 
    /// initially subscribe or there is a connection failure.
    void subscribe(Subscription &subscription);

    /// Here we attempt to retry connection if we are not connected. And if any of the subscriptions have failed to
    /// subscribe with the broker we also retry.
    void healthCheck();

    /*************************************************************************/
    /********** MEMBER CALLBACKS *********************************************/
    /*************************************************************************/

    /// This is called if the client detects a change in connection status (accepted, disconnect, or timeout) or 
    /// receives a rejection when trying to connect. We connect the remaining callbacks when the connection is 
    /// accepted.
    void connectionChanged(mqtt_connection_status_t status);

    /// The idea is that data for a new publication is going to be announced first and we can setup this module to 
    /// receive the payload later.
    void incomingPublish(const char *topic, uint32_t totalLength);

    /// Callback to receive a payload. Doesn't include fixed or variable header. Topic comes from last call of incoming 
    /// publish callback. This can be invoked multiple times to receive the full payload. It copies all data out and 
    /// checks your have received all data. If you haven't and `incomingPublish(..)` is called again, something went 
    /// wrong. Calls the subscriptions receievedPayload() callback when entire message has been received.
    ///
    /// @param data
    ///     Pointer to the latest received data portion of data for the payload only.
    /// @param len
    ///     The size of the received data for this call. Will only ever include the payload, never the header.
    /// @param flags
    ///     MQTT_DATA_FLAG_LAST if this is the last portion of data for this message else 0.
    void incomingData(const uint8_t *data, uint16_t len, uint8_t flags);

    /*************************************************************************/
    /********** STATIC CALLBACKS *********************************************/
    /*************************************************************************/

    static void connectionChangedCb(Client *client, void *arg, mqtt_connection_status_t status) {
        static_cast<void>(client);
        reinterpret_cast<MqttClient *>(arg)->connectionChanged(status);
    }

    static void incomingPublishCb(void *arg, const char *topic, u32_t totalLength) {
        reinterpret_cast<MqttClient *>(arg)->incomingPublish(topic, totalLength);
    }

    static void incomingDataCb(void *arg, const uint8_t *data, uint16_t len, uint8_t flags) {
        reinterpret_cast<MqttClient *>(arg)->incomingData(data, len, flags);
    }

    static void publishRequestCb(void *arg, err_t err) {
        reinterpret_cast<Publication *>(arg)->publicationRequest(err == ERR_OK);
    }

    static void subscriptionRequestCb(void *arg, err_t result) {
        // The result can be ERR_OK, ERR_TIMEOUT, or ERR_ABRT (if (un)subscription was denied). We just flag that 
        // the subscription is in error and the client will later try and re-subscribe.
        reinterpret_cast<Subscription *>(arg)->subscribed = result == ERR_OK;
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    SubscriptionMap mSubscriptions; 
    Config mConfig;
    lwipserver::utils::LoopTimer mHealthCheckTimer;
    Client *mLwIPClient{nullptr};
    Subscription *mActiveSubscription{nullptr};
    
}; // class MqttClient

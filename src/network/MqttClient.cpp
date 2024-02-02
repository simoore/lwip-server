#include <algorithm>
#include <functional>

#include "lwipserver/network/MqttClient.h"

namespace lwipserver::network {

bool MqttClient::init(const Config &cfg) {
    mConfig = cfg;
    mLwIPClient = mqtt_client_new();
    if (!mLwIPClient) {
        printf("MqttClient::init, failed to allocate memory for LwIP MQTT client.\n");
        return false;
    }
    mHealthCheckTimer.setExpiry(sHealthCheckExpiry);
    mHealthCheckTimer.registerCallback(std::bind(&MqttClient::healthCheck, this));
    return true;
}


bool MqttClient::registerSubscription(Subscription &subscription) {
    if (mSubscriptions.full()) {
        printf("MqttClient::subscribe, cannot accept any more subscriptions.\n");
        return false;
    }
    mSubscriptions.emplace(subscription.topic, &subscription);
    subscribe(subscription);
    return true;
}


void MqttClient::publish(Publication &publication) {
    err_t err = mqtt_publish(mLwIPClient, publication.topicName, publication.payload, 
        publication.payloadSize, sQos, sRemain, publishRequestCb, &publication);
    if (err != ERR_OK) {
        printf("MqttClient::publish, publish returned error code %d\n", err);
        publication.publicationRequest(false);
    }
}

/*************************************************************************/
/********** PRIVATE FUNCTIONS ********************************************/
/*************************************************************************/

void MqttClient::connect() {
    const ClientInfo clientInfo{
        .client_id = mConfig.clientId,
        .client_user = nullptr,
        .client_pass = nullptr,

        // This is the time interval at which the client sends a ping message to the broker to notify it is 
        // still available. Unit is seconds.
        .keep_alive = mConfig.keepAlive,

        // The will message is a message sent by the broker to other clients when this client disconnects 
        // ungracefully. Other will_* properties are ignored if this is nullptr.
        .will_topic = nullptr
    };
    err_t err = mqtt_client_connect(mLwIPClient, &mConfig.brokerIpAddr, mConfig.brokerPort, 
        connectionChangedCb, this, &clientInfo);
    if (err != ERR_OK) {
        // ERR_ISCONN, if already connected. ERR_VAL for various parameter issues, ERR_MEM if unable to allocated
        // memory.
        printf("MqttClient::connect, connection returned error code %d\n", err);
    }
} 


void MqttClient::subscribe(Subscription &subscription) {
    err_t err = mqtt_subscribe(mLwIPClient, subscription.topic, sQos, subscriptionRequestCb, &subscription);
    subscription.subscribed = err == ERR_OK;
    if (err != ERR_OK) {
        // Errors include: ERR_CONN if no tcp connection, ERR_MEM if cannot allocate memory for packet.
        printf("MqttClient::subscribe, return err %d\n", err);
    }
}


void MqttClient::healthCheck() {
    if (!connected()) {
        connect();
    }
    for (auto &[key, val] : mSubscriptions) {
        if (!val->subscribed) {
            subscribe(*val);
        }
    }
}

/*************************************************************************/
/********** MEMBER CALLBACKS *********************************************/
/*************************************************************************/

void MqttClient::connectionChanged(mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt_set_inpub_callback(mLwIPClient, incomingPublishCb, incomingDataCb, static_cast<void *>(this));
        for (auto &[key, val] : mSubscriptions) {
            subscribe(*val);
        }
    } else {
        printf("MqttClient::connectionChanged, connection failed, error code %d\n", status);
        mqtt_set_inpub_callback(mLwIPClient, nullptr, nullptr, nullptr);
    }
}


void MqttClient::incomingPublish(const char *topic, uint32_t totalLength) {
    // For every TCP packet that arrives, LwIP MQTT client copies the data into a buffer in `mqtt_parse_incoming`
    // and calls `mqtt_message_received` at the end. `client->rx_buffer` is the buffer `client->msg_idx` contains 
    // the length of the message in the buffer. Each time data is copied into the buffer `mqtt_message_received` 
    // is called with three length parameters: the length of the fixed header, the length of the amount of data 
    // copied in the variable part of the payload, and the remaining number of bytes expected.
    //
    // The whole MQTT message is left in this buffer until all has arrived and sent to the application, but only
    // portions of the payload are returned to the application via callbacks. Receiving of messages are not 
    // multiplexed. One whole message is received at one time.
    //
    // On receiving the first portion of the message, this incomingPublish callback is called with the topic name
    // and the total size of the payload.
    if (mActiveSubscription) {
        printf("MqttClient::incomingPublish, clearing an active subsciption, likely contains partial payload.\n");
    }
    mActiveSubscription = nullptr;

    auto it = mSubscriptions.find(SubscriptionKey(topic));
    if (it != mSubscriptions.end()) {
        mActiveSubscription = it->second;
        mActiveSubscription->size = 0;
        mActiveSubscription->totalSize = totalLength;
    } else {
        printf("MqttClient::incomingPublish, incoming topic not found.");
    }
}


void MqttClient::incomingData(const uint8_t *data, uint16_t len, uint8_t flags) {
    if (!mActiveSubscription) {
        printf("MqttClient::incomingData, no active subscription.\n");
        return;
    }

    const uint32_t bufSize = mActiveSubscription->capacity - mActiveSubscription->size;
    const uint32_t size = std::min(static_cast<uint32_t>(len), bufSize);
    std::copy_n(data, size, mActiveSubscription->buffer + mActiveSubscription->size);
    mActiveSubscription->size += size;

    if (size != len) {
        printf("MqttClient::incomingData, could not copy entire payload.\n");
        return;
    }

    if (flags & MQTT_DATA_FLAG_LAST) {
        if (mActiveSubscription->size != mActiveSubscription->totalSize) {
            printf("MqttClient::incomingData, missing portion of payload.\n");
            return;
        }
        if (mActiveSubscription->receivedPayload.is_valid()) {
            mActiveSubscription->receivedPayload();
        }
        mActiveSubscription = nullptr;
    }
}

} // namespace lwipserver::freertos
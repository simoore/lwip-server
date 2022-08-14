#include "lwip/apps/mqtt.h"

class MqttClient {
public:

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    struct Config {
        ip_addr_t brokerIpAddr;
        uint16_t brokerPort;
        uint16_t keepAlive;

        /// The client ID is what is used to identify the client on the network. It should be unique amounst all 
        /// clients connected to a broker.
        const char *clientId;
    };

    using ClientInfo = struct mqtt_connect_client_info_t;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    void init(const Config &cfg) {
        mConfig = cfg;
        mClient = mqtt_client_new();
    }

    // void publish(, data, size) {
    //     mqtt_publish(mClient, , , , )
    // }


    /// Once initialized 
    void service() {
        if (mClient) {
            // TODO: timeout to connect
            connect();
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Attempts to connect to the MQTT broker.
    void connect() {
        ClientInfo clientInfo = {
            .client_id = mConfig.clientId,
            .client_user = nullptr,
            .client_pass = nullptr,

            // This is the time interval at which the client sends a ping message to the broker to notify it is still
            // available.
            .keep_alive = mConfig.keepAlive,

            // The will message is a message sent by the broker to other clients when this client disconnects 
            // ungracefully.
            .will_topic = nullptr
        };
        err_t err = mqtt_client_connect(mClient, &mConfig.brokerIpAddr, mConfig.brokerPort, 
            connectionChangedCb, this, &clientInfo);
        if (err != ERR_OK) {
            printf("MqttClient::connect, connection returned error code.");
        }
    }

    void connectionChanged(mqtt_connection_status_t status) {
        if (status != MQTT_CONNECT_ACCEPTED) {
            printf("Connection not accepted, error code %d\n", status);
            return;
        }
        mConnected = true;
    }

    static void connectionChangedCb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
        static_cast<void>(client);
        reinterpret_cast<MqttClient *>(arg)->connectionChanged(status);
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    Config mConfig;
    mqtt_client_t *mClient{nullptr};
    bool mConnected{false};
    
};
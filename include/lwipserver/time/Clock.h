#include <string_view>

#include "lwipserver/utils/MainLoopTimer.h"
#include "lwipserver/network/MqttClient.h"


/// This class implements a real time clock that synchronize with a time server and broadcasts the time on a MQTT 
/// topic.
class Clock {
public:

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    static constexpr uint32_t sTimerExpiry = 2000;
    static constexpr const char *topic = "LwipServerClock";
    static constexpr std::string_view payload = "12:34:01";

    /// TODO: get rid of this system of passing references everywhere. THere is one MqttClient max, and one base max.
    Clock(MqttClient &client, IBase &base): mMqttClient(client), mTimer(base) {}

    void init() {
        mPublication.topicName = topic;
        mPublication.payload = payload.data();
        mPublication.payloadSize = payload.size();
        mPublication.publicationRequest = etl::delegate<void(bool)>::create<Clock::publicationRequest>();
        mTimer.setExpiry(sTimerExpiry);
        mTimer.registerCallback(MainLoopTimer::Callback::create([&](){
            if (mMqttClient.connected()) {
                mMqttClient.publish(mPublication);
            }
        }));
    }

    void service() {
        mTimer.poll();
    }

    static void publicationRequest(bool ok) {
        if (ok) {
            printf("Clock::publicationRequest, publication ok.\n");
        } else {
            printf("Clock::publicationRequest, publication not ok.\n");
        }
    }

private:

    MqttClient &mMqttClient;
    MqttClient::Publication mPublication;
    MainLoopTimer mTimer;
};

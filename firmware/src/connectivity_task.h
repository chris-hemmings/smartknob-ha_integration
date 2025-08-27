#pragma once

#include <Arduino.h>
#include "task.h"
#include "storage_manager.h"
#include "logger.h"

struct Message {
    String trigger_name;
    int trigger_value;
};
struct ReceivedMessage {
    String trigger_name;
    int trigger_value;
};

typedef std::function<void(const String&, int)> MqttMessageCallback;

class ConnectivityTask : public Task<ConnectivityTask> {
    friend class Task<ConnectivityTask>; // Allow base Task to invoke protected run()

public:
    ConnectivityTask(const uint8_t task_core);
    ~ConnectivityTask();

    void sendMqttMessage(Message message);
    void handleIncomingMessage(ReceivedMessage received_message);
    void setLogger(Logger* logger);
    void setMqttMessageCallback(MqttMessageCallback callback);

    // Device state getters/setters
    int getLightState() const { return light_state_; }
    int getLampsState() const { return lamps_state_; }
    int getFanState() const { return fan_state_; }
    int getBlindsState() const { return blinds_state_; }
    int getClimateState() const { return climate_state_; }
    void setLightState(int state);
    void setLampsState(int state);
    void setFanState(int state);
    void setBlindsState(int state);
    void setClimateState(int state);
    static ConnectivityTask* instance;
    // MQTT connection status
    bool isMqttConnected() const;
protected:
    void run();

private:
    QueueHandle_t queue_;
    static const int QUEUE_SIZE = 5;
    Logger* logger_;
    char buf_[512];
    void initWiFi();
    StorageManager storage_;
    void loadStoredStates();
    boolean reconnect();
    static void callback(char* topic, byte* payload, unsigned int length);
    void log(const char* msg);
    void handleCallback(char* topic, byte* payload, unsigned int length);
    void sendMqttKnobStateDiscoveryMsg();
    // Device states
    int light_state_ = 0;
    int lamps_state_ = 0;
    int fan_state_ = 0;
    int blinds_state_ = 0;
    int climate_state_ = 0;
    
    // MQTT message callback
    MqttMessageCallback mqtt_message_callback_ = nullptr;

};

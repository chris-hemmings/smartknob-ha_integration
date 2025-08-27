#include "connectivity_task.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "ArduinoJson.h"
#include "secrets.h"
#include "storage_manager.h"

WiFiClient client;
PubSubClient mqtt(client);

long lastReconnectAttempt = 0;

//String smartknob_id = String(SMARTKNOB_ID);
String smartknob_id;
String discovery_topic;
String action_topic;
String subscribe_topic;
ConnectivityTask* ConnectivityTask::instance = nullptr;

ConnectivityTask::ConnectivityTask(const uint8_t task_core) :
    Task("Connectivity", 4096, 1, task_core),
    light_state_(0),
    lamps_state_(0),
    fan_state_(0),
    blinds_state_(0),
    climate_state_(0) {
    storage_.init();
    queue_ = xQueueCreate(QUEUE_SIZE, sizeof(Message));
    assert(queue_ != NULL);
    instance = this; // Store this instance
    loadStoredStates();
}

ConnectivityTask::~ConnectivityTask() {}

boolean ConnectivityTask::reconnect() {
    bool connected = mqtt.connect(smartknob_id.c_str(), MQTT_USERNAME, MQTT_PW);
    if (connected) {
        snprintf(buf_, sizeof(buf_), "MQTT Connected...");
        log(buf_);
        snprintf(buf_, sizeof(buf_), "Subscribing to %s", subscribe_topic.c_str());
        log(buf_);
        mqtt.subscribe(subscribe_topic.c_str());
        sendMqttKnobStateDiscoveryMsg();
    } else {
        // Get the MQTT state and log the error code for debugging
        int mqttState = mqtt.state();
        snprintf(buf_, sizeof(buf_), "MQTT Connect Failed! Error Code: %d", mqttState);
        log(buf_);
    }

    return connected;
}

void ConnectivityTask::callback(char* topic, byte* payload, unsigned int length) {
    if (instance) { // Ensure instance exists
        instance->handleCallback(topic, payload, length);
    }
}

void ConnectivityTask::handleCallback(char* topic, byte* payload, unsigned int length) {
    snprintf(buf_, sizeof(buf_), "Message Received on Topic: %s", topic);
    log(buf_);

    // Log raw payload
    char payload_str[length + 1];
    memcpy(payload_str, payload, length);
    payload_str[length] = '\0';
    snprintf(buf_, sizeof(buf_), "Raw payload (%d bytes): %s", length, payload_str);
    log(buf_);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        snprintf(buf_, sizeof(buf_), "JSON Parse failed: %s", error.c_str());
        log(buf_);
        return;
    }
    
    snprintf(buf_, sizeof(buf_), "JSON parsed successfully");
    log(buf_);

    if (doc.containsKey("trigger_name") && doc.containsKey("trigger_value")) {
        ReceivedMessage msg;
        msg.trigger_name = doc["trigger_name"].as<String>();
        msg.trigger_value = doc["trigger_value"].as<int>();

        snprintf(buf_, sizeof(buf_), "Message validated - Name: %s, Value: %d", 
                 msg.trigger_name.c_str(), msg.trigger_value);
        log(buf_);

        // Update appropriate state based on trigger name
        if (msg.trigger_name == "light") {
            setLightState(msg.trigger_value);
        } else if (msg.trigger_name == "lamps") {
            setLampsState(msg.trigger_value);            
        } else if (msg.trigger_name == "fan") {
            setFanState(msg.trigger_value);
        } else if (msg.trigger_name == "blinds") {
            setBlindsState(msg.trigger_value);
        } else if (msg.trigger_name == "climate") {
            setClimateState(msg.trigger_value);
        }
        
        // Notify interface task about the MQTT message
        if (mqtt_message_callback_) {
            mqtt_message_callback_(msg.trigger_name, msg.trigger_value);
        }
    }
}

void ConnectivityTask::sendMqttKnobStateDiscoveryMsg() {
    DynamicJsonDocument payload(1024);
    discovery_topic = "homeassistant/device_automation/knob_" + smartknob_id + "/action_knob/config";
    action_topic = "knob/" + smartknob_id + "/action";
    subscribe_topic = "knob/" + smartknob_id + "/value";
    // Populate the JSON payload
    payload["automation_type"] = "trigger";
    payload["type"] = "action";
    payload["subtype"] = "knob_input";
    payload["topic"] = action_topic;

    JsonObject device = payload.createNestedObject("device");
    device["name"] = "SmartKnob";
    device["model"] = "Model 1";
    device["sw_version"] = "0.0.1";
    device["manufacturer"] = "lgc00";

    JsonArray identifiers = device.createNestedArray("identifiers");
    identifiers.add(smartknob_id);

    // Serialize JSON into a string
    std::string jsonString;
    serializeJson(payload, jsonString);

    // Log the JSON message
    snprintf(buf_, sizeof(buf_), "Serialized JSON (%d bytes): %s", jsonString.length(), jsonString.c_str());
    log(buf_);

    // Ensure MQTT is connected
    if (!mqtt.connected()) {
        log("MQTT not connected. Skipping discovery message.");
        return;
    }

    // Publish the message
    if (mqtt.publish(discovery_topic.c_str(), jsonString.c_str())) {
        snprintf(buf_, sizeof(buf_), "Discovery message sent successfully to %s", discovery_topic.c_str());
        log(buf_);
    } else {
        snprintf(buf_, sizeof(buf_), "Failed to send discovery message to %s", discovery_topic.c_str());
        log(buf_);
    }
}

void ConnectivityTask::run() {
    initWiFi();
    mqtt.setServer(MQTT_SERVER, MQTT_SERVERPORT);
    mqtt.setBufferSize(512);  // Adjust size as needed
    mqtt.setCallback(ConnectivityTask::callback); // Use the static method
    mqtt.loop();
    
    while (1) {
        // WiFi reconnection logic
        if (WiFi.status() != WL_CONNECTED) {
            snprintf(buf_, sizeof(buf_), "WiFi disconnected. Attempting to reconnect...");
            log(buf_);
            WiFi.reconnect();
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait a bit before next check
            continue; // Skip MQTT logic until WiFi is back
        }        
        if (!mqtt.connected()) {
            long now = millis();
            if (now - lastReconnectAttempt > 5000) {
                lastReconnectAttempt = now;
                // Attempt to reconnect
                if (reconnect()) {
                    lastReconnectAttempt = 0;
                }
            }
        } else {
            // Client connected
            mqtt.loop();
            Message message;
            if (xQueueReceive(queue_, &message, 0) == pdTRUE) {
                StaticJsonDocument<200> payload;
                char buffer[256];
                payload["trigger_name"] = message.trigger_name;
                payload["trigger_value"] = message.trigger_value;
                size_t n = serializeJson(payload, buffer);
                if (serializeJson(payload, buffer, sizeof(buffer)) > sizeof(buffer)) {
                    return;
                }
                mqtt.publish(action_topic.c_str(), buffer);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Adjust this delay as needed.
    }
  
}

void ConnectivityTask::sendMqttMessage(Message message) {
    xQueueSend(queue_, &message, portMAX_DELAY);
}

void ConnectivityTask::initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) { // 20 sec timeout
      snprintf(buf_, sizeof(buf_), "Connecting to WiFi...");
      log(buf_);
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED) {
      snprintf(buf_, sizeof(buf_), "%s", WiFi.localIP().toString().c_str());
      log(buf_);
      smartknob_id = WiFi.macAddress();
      smartknob_id.replace(":", "");  // Remove colons for a cleaner identifier
      log(("Device MAC Address: " + smartknob_id).c_str());
    } else {
      snprintf(buf_, sizeof(buf_), "Failed to connect to WiFi.");
      log(buf_);
    }
}

void ConnectivityTask::setLogger(Logger* logger) {
    logger_ = logger;
}

void ConnectivityTask::setMqttMessageCallback(MqttMessageCallback callback) {
    mqtt_message_callback_ = callback;
}

void ConnectivityTask::log(const char* msg) {
    if (logger_ != nullptr) {
        logger_->log(msg);
    }
}

void ConnectivityTask::loadStoredStates() {
    storage_.loadState("light", light_state_);
    storage_.loadState("lamps", lamps_state_);
    storage_.loadState("fan", fan_state_);
    storage_.loadState("blinds", blinds_state_);
    storage_.loadState("climate", climate_state_);
}

void ConnectivityTask::setLightState(int state) {
    light_state_ = state;
    storage_.saveState("light", state);
}

void ConnectivityTask::setLampsState(int state) {
    lamps_state_ = state;
    storage_.saveState("lamps", state);
}

void ConnectivityTask::setFanState(int state) {
    fan_state_ = state;
    storage_.saveState("fan", state);
}

void ConnectivityTask::setBlindsState(int state) {
    blinds_state_ = state;
    storage_.saveState("blinds", state);
}

void ConnectivityTask::setClimateState(int state) {
    climate_state_ = state;
    storage_.saveState("climate", state);
}

bool ConnectivityTask::isMqttConnected() const {
    return mqtt.connected();
}

// Device state variables
int light_state_;
int lamps_state_;
int fan_state_;
int blinds_state_;
int climate_state_;

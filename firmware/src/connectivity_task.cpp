#include "connectivity_task.h"
#include <WiFi.h>
#include "PubSubClient.h"
#include "ArduinoJson.h"
#include "secrets.h"
#include "storage_manager.h"
#include "home_screen_data.h"
#include "weather_util.h"
#include "http_util.h"

#include <time.h>

WiFiClient client;
PubSubClient mqtt(client);

long lastReconnectAttempt = 0;

//String smartknob_id = String(SMARTKNOB_ID);
String smartknob_id;
String discovery_topic;
String action_topic;
String subscribe_topic;
ConnectivityTask* ConnectivityTask::instance = nullptr;

void ConnectivityTask::reportWeatherStatus(const char* log_msg, const char* screen_line) {
    log(log_msg);
    static uint32_t status_version = 1000;
    status_version++;
    homeScreenDataUpdateWeather(screen_line, "\ue2bd", status_version);
}

ConnectivityTask::ConnectivityTask(const uint8_t task_core) :
    Task("Connectivity", 10240, 1, task_core),
    light_state_(0),
    lamps_state_(0),
    fan_state_(0),
    blinds_state_(0),
    climate_state_(0) {
    storage_.init();
    queue_ = xQueueCreate(QUEUE_SIZE, sizeof(Message));
    assert(queue_ != NULL);
    instance = this; // Store this instance
#if defined(SMARTKNOB_ID)
    smartknob_id = SMARTKNOB_ID;
#endif
    loadStoredStates();
}

ConnectivityTask::~ConnectivityTask() {}

static const char* mqttStateToString(const int state) {
    switch (state) {
        case -4: return "connection timeout";
        case -3: return "connection lost";
        case -2: return "connect failed";
        case -1: return "disconnected";
        case 1: return "bad protocol";
        case 2: return "bad client id";
        case 3: return "broker unavailable";
        case 4: return "bad credentials";
        case 5: return "not authorized";
        default: return "unknown";
    }
}

void ConnectivityTask::setupMqttTopics() {
    if (smartknob_id.length() == 0) {
        return;
    }
    discovery_topic = "homeassistant/device_automation/knob_" + smartknob_id + "/action_knob/config";
    action_topic = "knob/" + smartknob_id + "/action";
    subscribe_topic = "knob/" + smartknob_id + "/value";
}

boolean ConnectivityTask::reconnect() {
    setupMqttTopics();
    snprintf(buf_, sizeof(buf_), "MQTT connecting to %s:%d as client '%s'...",
             MQTT_SERVER, MQTT_SERVERPORT, smartknob_id.c_str());
    log(buf_);

    const bool connected = mqtt.connect(smartknob_id.c_str(), MQTT_USERNAME, MQTT_PW);
    if (connected) {
        snprintf(buf_, sizeof(buf_), "MQTT connected to %s:%d", MQTT_SERVER, MQTT_SERVERPORT);
        log(buf_);
        snprintf(buf_, sizeof(buf_), "Subscribing to %s", subscribe_topic.c_str());
        log(buf_);
        mqtt.subscribe(subscribe_topic.c_str());
        sendMqttKnobStateDiscoveryMsg();
    } else {
        const int mqtt_state = mqtt.state();
        snprintf(buf_, sizeof(buf_), "MQTT connect failed (%s, code %d) -> %s:%d",
                 mqttStateToString(mqtt_state), mqtt_state, MQTT_SERVER, MQTT_SERVERPORT);
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
    setupMqttTopics();
    DynamicJsonDocument payload(1024);
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

void ConnectivityTask::syncNetworkTime() {
    if (time_sync_started_) {
        return;
    }
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    time_sync_started_ = true;
    log("NTP time sync started");
}

void ConnectivityTask::fetchGeoLocation() {
    if (WiFi.status() != WL_CONNECTED) {
        reportWeatherStatus("WEATHER: skip geo (no WiFi)", "No WiFi");
        return;
    }

    log("WEATHER: fetching location...");
    reportWeatherStatus("WEATHER: fetching location", "Locating...");

    float latitude = 0;
    float longitude = 0;
    int32_t offset_sec = 0;
    char timezone_name[32] = {};

#if defined(WEATHER_USE_MANUAL_LOCATION)
    latitude = WEATHER_LATITUDE;
    longitude = WEATHER_LONGITUDE;
#endif

    String payload;
    if (httpGetUrl("http://ip-api.com/json/?fields=status,message,lat,lon,timezone,offset,city", payload)) {
        StaticJsonDocument<512> doc;
        const DeserializationError error = deserializeJson(doc, payload);
        if (!error && doc["status"] == "success") {
#if !defined(WEATHER_USE_MANUAL_LOCATION)
            latitude = doc["lat"] | 0.0f;
            longitude = doc["lon"] | 0.0f;
#endif
            offset_sec = doc["offset"] | 0;
            const char* tz = doc["timezone"] | "";
            const char* city = doc["city"] | "";
            if (city[0] != '\0') {
                strncpy(timezone_name, city, sizeof(timezone_name) - 1);
            } else if (tz[0] != '\0') {
                strncpy(timezone_name, tz, sizeof(timezone_name) - 1);
            }
        } else {
            snprintf(buf_, sizeof(buf_), "Geo JSON failed: %s", error.c_str());
            log(buf_);
        }
    } else {
        log("WEATHER: ip-api.com HTTP failed, trying ipapi.co");
        if (httpGetUrl("https://ipapi.co/json/", payload)) {
            StaticJsonDocument<512> doc;
            if (!deserializeJson(doc, payload)) {
#if !defined(WEATHER_USE_MANUAL_LOCATION)
                latitude = doc["latitude"] | 0.0f;
                longitude = doc["longitude"] | 0.0f;
#endif
                const char* raw_offset = doc["utc_offset"] | "";
                if (strlen(raw_offset) >= 5 && (raw_offset[0] == '+' || raw_offset[0] == '-')) {
                    const int sign = (raw_offset[0] == '-') ? -1 : 1;
                    const int hours = (raw_offset[1] - '0') * 10 + (raw_offset[2] - '0');
                    const int minutes = (raw_offset[3] - '0') * 10 + (raw_offset[4] - '0');
                    offset_sec = sign * (hours * 3600 + minutes * 60);
                }
                const char* city = doc["city"] | "";
                if (city[0] != '\0') {
                    strncpy(timezone_name, city, sizeof(timezone_name) - 1);
                }
            }
        }
    }

    String tz_payload;
    if (!tz_payload.length()) {
        httpGetUrl("http://worldtimeapi.org/api/ip", tz_payload);
    }
    if (!tz_payload.length()) {
        httpGetUrl("https://worldtimeapi.org/api/ip", tz_payload);
    }
    if (tz_payload.length()) {
        StaticJsonDocument<768> tz_doc;
        if (!deserializeJson(tz_doc, tz_payload)) {
            if (offset_sec == 0) {
                const char* raw_offset = tz_doc["utc_offset"] | "+00:00";
                int sign = 1;
                if (raw_offset[0] == '-') {
                    sign = -1;
                }
                int hours = 0;
                int minutes = 0;
                sscanf(raw_offset + 1, "%d:%d", &hours, &minutes);
                offset_sec = sign * (hours * 3600 + minutes * 60);
            }
#if !defined(WEATHER_USE_MANUAL_LOCATION)
            if (latitude == 0.0f && longitude == 0.0f) {
                latitude = tz_doc["latitude"] | 0.0f;
                longitude = tz_doc["longitude"] | 0.0f;
            }
#endif
        }
    }

#if !defined(WEATHER_USE_MANUAL_LOCATION)
    if (latitude == 0.0f && longitude == 0.0f) {
        reportWeatherStatus("WEATHER: geo failed (no lat/lon)", "Geo failed");
        return;
    }
#endif

    if (offset_sec == 0) {
        log("WEATHER: timezone offset missing; using UTC for local time");
    }

    location_latitude_ = latitude;
    location_longitude_ = longitude;
    utc_offset_sec_ = offset_sec;
    geo_location_valid_ = true;
    homeScreenDataUpdateGeo(latitude, longitude, offset_sec, timezone_name);
    snprintf(buf_, sizeof(buf_), "WEATHER: geo OK %.4f, %.4f TZ %+ld sec (%s)",
             latitude, longitude, (long)offset_sec, timezone_name);
    log(buf_);
}

void ConnectivityTask::fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) {
        reportWeatherStatus("WEATHER: skip forecast (no WiFi)", "No WiFi");
        return;
    }

    if (!geo_location_valid_) {
        fetchGeoLocation();
        if (!geo_location_valid_) {
            return;
        }
    }

    log("WEATHER: fetching forecast...");
    reportWeatherStatus("WEATHER: fetching forecast", "Loading weather...");

    char url[384];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,weather_code&temperature_unit=celsius",
             location_latitude_, location_longitude_);

    String payload;
    bool fetched = httpGetUrl(url, payload);
    if (!fetched) {
        snprintf(url, sizeof(url),
                 "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,weather_code&temperature_unit=celsius",
                 location_latitude_, location_longitude_);
        fetched = httpGetUrl(url, payload);
    }
    if (!fetched) {
        snprintf(buf_, sizeof(buf_), "WEATHER: open-meteo failed (try %u)", weather_fail_count_ + 1);
        log(buf_);
        weather_fail_count_++;
        reportWeatherStatus(buf_, "Weather unavailable");
        return;
    }

    snprintf(buf_, sizeof(buf_), "WEATHER: got %u bytes from open-meteo", payload.length());
    log(buf_);

    float temp_c = 0.0f;
    int weather_code = -1;
    if (!weatherParseOpenMeteoResponse(payload.c_str(), &temp_c, &weather_code)) {
        snprintf(buf_, sizeof(buf_), "WEATHER: parse failed (first bytes: %.40s)", payload.c_str());
        log(buf_);
        weather_fail_count_++;
        reportWeatherStatus(buf_, "Bad weather data");
        return;
    }

    char description[32];
    char icon[8];
    weatherFromWmoCode(weather_code, description, sizeof(description), icon, sizeof(icon));

    char line[32];
    snprintf(line, sizeof(line), "%.0f°C  %s", temp_c, description);

    weather_fail_count_ = 0;
    weather_fetch_version_++;
    homeScreenDataUpdateWeather(line, icon, weather_fetch_version_);
    snprintf(buf_, sizeof(buf_), "WEATHER: OK %s", line);
    log(buf_);
}

void ConnectivityTask::run() {
    homeScreenDataInit();
    initWiFi();
    if (WiFi.status() == WL_CONNECTED) {
        syncNetworkTime();
        fetchGeoLocation();
        last_geo_fetch_ms_ = millis();
        fetchWeather();
        last_weather_fetch_ms_ = millis();
    }
    mqtt.setServer(MQTT_SERVER, MQTT_SERVERPORT);
    mqtt.setBufferSize(512);  // Adjust size as needed
    mqtt.setCallback(ConnectivityTask::callback); // Use the static method
    mqtt.loop();
    
    while (1) {
        // WiFi reconnection logic
        if (WiFi.status() != WL_CONNECTED) {
            snprintf(buf_, sizeof(buf_), "WiFi disconnected. Attempting to reconnect...");
            log(buf_);
            geo_location_valid_ = false;
            WiFi.reconnect();
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait a bit before next check
            continue; // Skip MQTT logic until WiFi is back
        }

        if (!time_sync_started_) {
            syncNetworkTime();
        }

        if (last_geo_fetch_ms_ == 0 || millis() - last_geo_fetch_ms_ >= GEO_FETCH_INTERVAL_MS) {
            fetchGeoLocation();
            last_geo_fetch_ms_ = millis();
        }

        const uint32_t weather_interval = weather_fail_count_ > 0
            ? WEATHER_RETRY_INTERVAL_MS
            : WEATHER_FETCH_INTERVAL_MS;
        if (last_weather_fetch_ms_ == 0 || millis() - last_weather_fetch_ms_ >= weather_interval) {
            fetchWeather();
            last_weather_fetch_ms_ = millis();
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
      snprintf(buf_, sizeof(buf_), "WiFi IP: %s", WiFi.localIP().toString().c_str());
      log(buf_);
      syncNetworkTime();
#if defined(SMARTKNOB_ID) && (SMARTKNOB_ID[0] != '\0')
      smartknob_id = SMARTKNOB_ID;
      snprintf(buf_, sizeof(buf_), "MQTT client id: %s (from SMARTKNOB_ID)", smartknob_id.c_str());
#else
      smartknob_id = WiFi.macAddress();
      smartknob_id.replace(":", "");
      snprintf(buf_, sizeof(buf_), "MQTT client id: %s (from MAC)", smartknob_id.c_str());
#endif
      log(buf_);
      setupMqttTopics();
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

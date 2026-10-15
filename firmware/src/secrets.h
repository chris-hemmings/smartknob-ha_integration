#define WIFI_SSID "Pretty_Fly_For_A_Wifi"
#define WIFI_PASSWORD "9306023373"

#define MQTT_SERVER        "192.168.0.132"
#define MQTT_SERVERPORT    1883
#define MQTT_USERNAME      "mqtt"
#define MQTT_PW            "M3tall1ca."
#define MQTT_COMMAND_TOPIC ""

#define SMARTKNOB_ID "sk1" // id used for MQTT autodiscovery - needs to be unique for each device

// Home screen: local weather via Open-Meteo (https://open-meteo.com/) and NTP time.
#ifndef WEATHER_LATITUDE
#define WEATHER_LATITUDE 40.7128f
#endif
#ifndef WEATHER_LONGITUDE
#define WEATHER_LONGITUDE -74.0060f
#endif
#ifndef WEATHER_TIMEZONE
#define WEATHER_TIMEZONE "EST5EDT,M3.2.0,M11.1.0"
#endif
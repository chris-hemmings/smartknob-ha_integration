#include "home_screen_data.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>

static HomeScreenData data_ = {};
static SemaphoreHandle_t mutex_;

void homeScreenDataInit() {
    mutex_ = xSemaphoreCreateMutex();
    memset(&data_, 0, sizeof(data_));
    strncpy(data_.time_line, "--:--", sizeof(data_.time_line));
    strncpy(data_.date_line, "", sizeof(data_.date_line));
    strncpy(data_.weather_temp, "--", sizeof(data_.weather_temp));
    strncpy(data_.weather_desc, "Loading weather", sizeof(data_.weather_desc));
    data_.weather_code = -1;
    data_.utc_offset_sec = 0;
    data_.geo_valid = false;
}

void homeScreenDataRead(HomeScreenData* out) {
    if (out == nullptr || mutex_ == nullptr) {
        return;
    }
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        *out = data_;
        xSemaphoreGive(mutex_);
    }
}

void homeScreenDataWrite(const HomeScreenData* in) {
    if (in == nullptr || mutex_ == nullptr) {
        return;
    }
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        data_ = *in;
        xSemaphoreGive(mutex_);
    }
}

void homeScreenDataUpdateWeather(const char* temp_line, const char* desc_line, uint32_t version,
                                 int16_t weather_code) {
    if (mutex_ == nullptr) {
        return;
    }
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (temp_line != nullptr) {
            strncpy(data_.weather_temp, temp_line, sizeof(data_.weather_temp) - 1);
            data_.weather_temp[sizeof(data_.weather_temp) - 1] = '\0';
        }
        if (desc_line != nullptr) {
            strncpy(data_.weather_desc, desc_line, sizeof(data_.weather_desc) - 1);
            data_.weather_desc[sizeof(data_.weather_desc) - 1] = '\0';
        }
        data_.weather_code = weather_code;
        data_.weather_valid = true;
        data_.weather_version = version;
        xSemaphoreGive(mutex_);
    }
}

void homeScreenDataUpdateGeo(float latitude, float longitude, int32_t utc_offset_sec, const char* timezone_label) {
    if (mutex_ == nullptr) {
        return;
    }
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        data_.latitude = latitude;
        data_.longitude = longitude;
        data_.utc_offset_sec = utc_offset_sec;
        data_.geo_valid = true;
        data_.geo_version++;
        if (timezone_label != nullptr) {
            strncpy(data_.timezone_label, timezone_label, sizeof(data_.timezone_label) - 1);
            data_.timezone_label[sizeof(data_.timezone_label) - 1] = '\0';
        }
        xSemaphoreGive(mutex_);
    }
}

void homeScreenDataUpdateTime(const char* time_line, const char* date_line, bool valid) {
    if (mutex_ == nullptr) {
        return;
    }
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        if (time_line != nullptr) {
            strncpy(data_.time_line, time_line, sizeof(data_.time_line) - 1);
            data_.time_line[sizeof(data_.time_line) - 1] = '\0';
        }
        if (date_line != nullptr) {
            strncpy(data_.date_line, date_line, sizeof(data_.date_line) - 1);
            data_.date_line[sizeof(data_.date_line) - 1] = '\0';
        }
        data_.time_valid = valid;
        xSemaphoreGive(mutex_);
    }
}

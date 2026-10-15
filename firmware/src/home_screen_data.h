#pragma once

#include <stddef.h>
#include <stdint.h>

struct HomeScreenData {
    char time_line[16];
    char date_line[24];
    char weather_line[40];
    char weather_icon[8];
    char timezone_label[32];
    bool time_valid;
    bool weather_valid;
    bool geo_valid;
    int32_t utc_offset_sec;
    float latitude;
    float longitude;
    uint32_t weather_version;
    uint32_t geo_version;
};

void homeScreenDataInit();
void homeScreenDataRead(HomeScreenData* out);
void homeScreenDataWrite(const HomeScreenData* data);
void homeScreenDataUpdateWeather(const char* weather_line, const char* weather_icon, uint32_t version);
void homeScreenDataUpdateTime(const char* time_line, const char* date_line, bool valid);
void homeScreenDataUpdateGeo(float latitude, float longitude, int32_t utc_offset_sec, const char* timezone_label);

#pragma once

#include <stddef.h>
#include <stdint.h>

struct HomeScreenData {
    char time_line[16];
    char date_line[24];
    char weather_line[32];
    char weather_icon[8];
    bool time_valid;
    bool weather_valid;
    uint32_t weather_version;
};

void homeScreenDataInit();
void homeScreenDataRead(HomeScreenData* out);
void homeScreenDataWrite(const HomeScreenData* data);
void homeScreenDataUpdateWeather(const char* weather_line, const char* weather_icon, uint32_t version);
void homeScreenDataUpdateTime(const char* time_line, const char* date_line, bool valid);

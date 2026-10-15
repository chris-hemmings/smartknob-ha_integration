#pragma once

#include <stddef.h>
#include <stdint.h>

void weatherFromWmoCode(int code, char* description_out, size_t description_len, char* icon_out, size_t icon_len);

// Parses Open-Meteo forecast JSON (ArduinoJson first, substring fallback).
bool weatherParseOpenMeteoResponse(const char* json, float* temp_c_out, int* weather_code_out);

bool weatherParseOpenMeteoTimezone(const char* json, int32_t* utc_offset_sec_out, char* tz_label_out,
                                   size_t tz_label_len);

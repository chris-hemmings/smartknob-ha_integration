#pragma once

#include <Arduino.h>

// Full URL, e.g. "https://api.open-meteo.com/v1/forecast?..."
bool httpGetUrl(const char* url, String& body_out, uint32_t timeout_ms = 15000, int* http_status_out = nullptr);

#pragma once

#include <Arduino.h>

// Full URL, e.g. "https://api.open-meteo.com/v1/forecast?..."
bool httpGetUrl(const char* url, String& body_out, uint32_t timeout_ms = 15000, int* http_status_out = nullptr);

// HTTP GET with explicit Host header (use connect_ip when DNS fails but you know the server IP).
bool httpGetHostPath(const char* host, uint16_t port, const char* path, String& body_out, uint32_t timeout_ms = 15000,
                     int* http_status_out = nullptr, const char* connect_ip = nullptr);

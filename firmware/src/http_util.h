#pragma once

#include <Arduino.h>

bool httpGet(const char* host, uint16_t port, const char* path, String& body_out, uint32_t timeout_ms = 8000);

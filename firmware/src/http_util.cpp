#include "http_util.h"

#include <WiFi.h>

bool httpGet(const char* host, uint16_t port, const char* path, String& body_out, uint32_t timeout_ms) {
    body_out = "";
    WiFiClient http_client;
    if (!http_client.connect(host, port)) {
        return false;
    }

    http_client.printf("GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

    bool headers_done = false;
    const uint32_t start_ms = millis();
    while (http_client.connected() && millis() - start_ms < timeout_ms) {
        while (http_client.available()) {
            const String line = http_client.readStringUntil('\n');
            if (!headers_done) {
                if (line == "\r") {
                    headers_done = true;
                }
                continue;
            }
            body_out += line;
        }
    }
    http_client.stop();
    return body_out.length() > 0;
}

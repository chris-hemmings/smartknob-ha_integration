#include "http_util.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static bool trimJsonPayload(String& body_out) {
    const int start = body_out.indexOf('{');
    if (start < 0) {
        return false;
    }
    if (start > 0) {
        body_out = body_out.substring(start);
    }
    return true;
}

static uint32_t clientTimeoutSeconds(uint32_t timeout_ms) {
    uint32_t sec = (timeout_ms + 999) / 1000;
    if (sec < 5) {
        sec = 5;
    }
    if (sec > 60) {
        sec = 60;
    }
    return sec;
}

bool httpGetUrl(const char* url, String& body_out, uint32_t timeout_ms, int* http_status_out) {
    body_out = "";
    if (http_status_out != nullptr) {
        *http_status_out = 0;
    }
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    const uint32_t client_timeout_sec = clientTimeoutSeconds(timeout_ms);

    HTTPClient http;
    http.setTimeout(timeout_ms);
    http.setConnectTimeout(timeout_ms);
    http.setUserAgent("SmartKnob/1.0");
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setReuse(false);

    WiFiClient plain_client;
    WiFiClientSecure secure_client;
    const bool use_tls = (strncmp(url, "https://", 8) == 0);

    bool began = false;
    if (use_tls) {
        secure_client.setInsecure();
        secure_client.setHandshakeTimeout(client_timeout_sec * 1000);
        secure_client.setTimeout(client_timeout_sec);
        began = http.begin(secure_client, url);
    } else {
        plain_client.setTimeout(client_timeout_sec);
        began = http.begin(plain_client, url);
    }

    if (!began) {
        if (http_status_out != nullptr) {
            *http_status_out = -1;
        }
        http.end();
        return false;
    }

    const int status = http.GET();
    if (http_status_out != nullptr) {
        *http_status_out = status;
    }
    if (status != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    body_out = http.getString();
    http.end();

    if (!trimJsonPayload(body_out)) {
        return false;
    }

    return body_out.length() > 0;
}

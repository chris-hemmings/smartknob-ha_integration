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

bool httpGetUrl(const char* url, String& body_out, uint32_t timeout_ms) {
    body_out = "";
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    HTTPClient http;
    http.setTimeout(timeout_ms);
    http.setUserAgent("SmartKnob/1.0");
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    WiFiClientSecure secure_client;
    const bool use_tls = (strncmp(url, "https://", 8) == 0);

    bool began = false;
    if (use_tls) {
        secure_client.setInsecure();
        began = http.begin(secure_client, url);
    } else {
        began = http.begin(url);
    }

    if (!began) {
        http.end();
        return false;
    }

    const int status = http.GET();
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

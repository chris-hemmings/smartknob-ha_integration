#include "weather_util.h"

#include <stdio.h>
#include <string.h>

void weatherFromWmoCode(int code, char* description_out, size_t description_len, char* icon_out, size_t icon_len) {
    const char* description = "Unknown";
    const char* icon = "\ue2bd";  // cloud

    switch (code) {
        case 0:
            description = "Clear sky";
            icon = "\ue430";
            break;
        case 1:
            description = "Mainly clear";
            icon = "\ue3c9";
            break;
        case 2:
            description = "Partly cloudy";
            icon = "\ue3c9";
            break;
        case 3:
            description = "Overcast";
            icon = "\ue2bd";
            break;
        case 45:
        case 48:
            description = "Foggy";
            icon = "\ue98f";
            break;
        case 51:
        case 53:
        case 55:
            description = "Drizzle";
            icon = "\ue0f5";
            break;
        case 56:
        case 57:
            description = "Freezing drizzle";
            icon = "\ue0f5";
            break;
        case 61:
        case 63:
        case 65:
            description = "Rain";
            icon = "\ue0f5";
            break;
        case 66:
        case 67:
            description = "Freezing rain";
            icon = "\ue0f5";
            break;
        case 71:
        case 73:
        case 75:
            description = "Snow";
            icon = "\ue2dc";
            break;
        case 77:
            description = "Snow grains";
            icon = "\ue2dc";
            break;
        case 80:
        case 81:
        case 82:
            description = "Rain showers";
            icon = "\uf070";
            break;
        case 85:
        case 86:
            description = "Snow showers";
            icon = "\ue2dc";
            break;
        case 95:
            description = "Thunderstorm";
            icon = "\ueb81";
            break;
        case 96:
        case 99:
            description = "Thunderstorm + hail";
            icon = "\ueb81";
            break;
        default:
            break;
    }

    if (description_out != nullptr && description_len > 0) {
        strncpy(description_out, description, description_len - 1);
        description_out[description_len - 1] = '\0';
    }
    if (icon_out != nullptr && icon_len > 0) {
        strncpy(icon_out, icon, icon_len - 1);
        icon_out[icon_len - 1] = '\0';
    }
}

#pragma once

#include "lvgl.h"

// Draw a colorful emoji-style weather icon for an Open-Meteo WMO weather code.
void weather_icon_draw(lv_obj_t* canvas, int wmo_code);

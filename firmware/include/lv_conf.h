/**
 * LVGL configuration for SmartKnob (PlatformIO / ESP32).
 * Included via -DLV_CONF_INCLUDE_SIMPLE and firmware/include.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif
#define LV_THEME_DEFAULT_DARK 1
#define LV_USE_LABEL 1
#define LV_USE_LOG 1
#define LV_USE_PERF_MONITOR 0
#define LV_FONT_FMT_TXT_LARGE 1

/* Not used on device; keep off so Windows builds skip heavy extras. */
#define LV_USE_FFMPEG 0
#define LV_USE_FREETYPE 0

#endif /* LV_CONF_H */

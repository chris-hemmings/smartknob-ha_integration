#include "home_view.h"

#include <esp_heap_caps.h>
#include <time.h>

#include "../home_screen_data.h"
#include "styles.h"
#include "view_util.h"
#include "weather_icon_draw.h"

static const int WEATHER_ICON_SIZE = 48;
// Left-edge positions on the 240x240 round display (y offset from screen vertical center).
static const lv_coord_t WEATHER_ICON_LEFT = 26;
static const lv_coord_t WEATHER_ICON_Y = 52;
static const lv_coord_t WEATHER_TEXT_LEFT = 82;
static const lv_coord_t WEATHER_TEMP_Y = 38;
static const lv_coord_t WEATHER_DESC_Y = 58;
static const lv_coord_t WEATHER_DESC_WIDTH = 108;

void HomeView::alignWeatherRow() {
    if (weather_icon_canvas_ != nullptr) {
        lv_obj_align(weather_icon_canvas_, LV_ALIGN_LEFT_MID, WEATHER_ICON_LEFT, WEATHER_ICON_Y);
    }
    if (label_weather_temp_ != nullptr) {
        lv_obj_align(label_weather_temp_, LV_ALIGN_LEFT_MID, WEATHER_TEXT_LEFT, WEATHER_TEMP_Y);
    }
    if (label_weather_desc_ != nullptr) {
        lv_obj_align(label_weather_desc_, LV_ALIGN_LEFT_MID, WEATHER_TEXT_LEFT, WEATHER_DESC_Y);
    }
}

void HomeView::ensureWeatherIconCanvas() {
    if (weather_icon_canvas_ != nullptr) {
        return;
    }

    weather_icon_buf_ = static_cast<lv_color_t*>(heap_caps_malloc(
        LV_CANVAS_BUF_SIZE_TRUE_COLOR(WEATHER_ICON_SIZE, WEATHER_ICON_SIZE) * sizeof(lv_color_t),
        MALLOC_CAP_8BIT));
    if (weather_icon_buf_ == nullptr) {
        return;
    }

    weather_icon_canvas_ = lv_canvas_create(screen_);
    lv_canvas_set_buffer(weather_icon_canvas_, weather_icon_buf_, WEATHER_ICON_SIZE, WEATHER_ICON_SIZE,
                         LV_IMG_CF_TRUE_COLOR);
    alignWeatherRow();
}

void HomeView::showWeatherIcon(int16_t wmo_code) {
    ensureWeatherIconCanvas();
    if (weather_icon_canvas_ == nullptr) {
        return;
    }
    weather_icon_draw(weather_icon_canvas_, wmo_code);
    lv_obj_clear_flag(weather_icon_canvas_, LV_OBJ_FLAG_HIDDEN);
    last_weather_code_ = wmo_code;
}

void HomeView::setupView(PB_SmartKnobConfig config) {
    last_weather_version_ = UINT32_MAX;
    last_time_refresh_ms_ = 0;
    weather_icon_canvas_ = nullptr;
    weather_icon_buf_ = nullptr;
    last_weather_code_ = -999;

    reset_screen_background();

    label_time_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_time_, &roboto_light_60, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_time_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_time_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, -54);

    label_date_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_date_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_date_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_date_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_date_, LV_ALIGN_CENTER, 0, -6);

    label_weather_temp_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_weather_temp_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_weather_temp_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_weather_temp_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(label_weather_temp_, 72);
    lv_label_set_long_mode(label_weather_temp_, LV_LABEL_LONG_CLIP);

    label_weather_desc_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_weather_desc_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_weather_desc_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_weather_desc_, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_width(label_weather_desc_, WEATHER_DESC_WIDTH);
    lv_label_set_long_mode(label_weather_desc_, LV_LABEL_LONG_WRAP);

    alignWeatherRow();

    lv_label_set_text(label_time_, "--:--");
    lv_label_set_text(label_date_, "Turn knob for menu");
    lv_label_set_text(label_weather_temp_, "--");
    lv_label_set_text(label_weather_desc_, "Loading weather");
    showWeatherIcon(-1);

    HomeScreenData cached = {};
    homeScreenDataRead(&cached);
    if (cached.weather_valid) {
        showWeatherIcon(cached.weather_code);
        lv_label_set_text(label_weather_temp_, cached.weather_temp);
        lv_label_set_text(label_weather_desc_, cached.weather_desc);
        last_weather_version_ = cached.weather_version;
    }
    if (cached.time_valid) {
        lv_label_set_text(label_time_, cached.time_line);
        lv_label_set_text(label_date_, cached.date_line);
    }

    lv_task_handler();
}

void HomeView::updateView(PB_SmartKnobState state) {
    reset_screen_background();

    const uint32_t now = millis();
    if (now - last_time_refresh_ms_ >= 1000) {
        last_time_refresh_ms_ = now;

        HomeScreenData geo = {};
        homeScreenDataRead(&geo);

        time_t utc = 0;
        time(&utc);
        if (geo.geo_valid && utc > 0) {
            const time_t local_epoch = utc + geo.utc_offset_sec;
            struct tm timeinfo;
            gmtime_r(&local_epoch, &timeinfo);

            char time_buf[16];
            const char colon = (timeinfo.tm_sec % 2 == 0) ? ':' : ' ';
            snprintf(time_buf, sizeof(time_buf), "%02d%c%02d", timeinfo.tm_hour, colon, timeinfo.tm_min);
            lv_label_set_text(label_time_, time_buf);

            char date_buf[24];
            strftime(date_buf, sizeof(date_buf), "%a, %b %d", &timeinfo);
            lv_label_set_text(label_date_, date_buf);

            homeScreenDataUpdateTime(time_buf, date_buf, true);
        } else if (utc > 0) {
            lv_label_set_text(label_time_, "--:--");
            lv_label_set_text(label_date_, "Turn knob for menu");
            homeScreenDataUpdateTime("--:--", "Turn knob for menu", false);
        } else {
            lv_label_set_text(label_time_, "--:--");
            lv_label_set_text(label_date_, "Syncing time...");
            homeScreenDataUpdateTime("--:--", "Syncing time...", false);
        }
    }

    HomeScreenData weather = {};
    homeScreenDataRead(&weather);
    if (weather.weather_version != last_weather_version_
        || weather.weather_code != last_weather_code_) {
        last_weather_version_ = weather.weather_version;
        lv_label_set_text(label_weather_temp_, weather.weather_temp);
        lv_label_set_text(label_weather_desc_, weather.weather_desc);
        showWeatherIcon(weather.weather_code);
        alignWeatherRow();
    }
}

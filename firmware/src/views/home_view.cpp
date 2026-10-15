#include "home_view.h"

#include <esp_heap_caps.h>
#include <time.h>

#include "../home_screen_data.h"
#include "styles.h"
#include "view_util.h"
#include "weather_icon_draw.h"

static const int WEATHER_ICON_SIZE = 52;

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
    lv_obj_align(weather_icon_canvas_, LV_ALIGN_CENTER, 0, 26);
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

    label_weather_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_weather_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_weather_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_weather_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(label_weather_, (TFT_WIDTH * 4) / 5);
    lv_label_set_long_mode(label_weather_, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_weather_, LV_ALIGN_CENTER, 0, 64);

    lv_label_set_text(label_time_, "--:--");
    lv_label_set_text(label_date_, "Turn knob for menu");
    lv_label_set_text(label_weather_, "Loading weather");
    showWeatherIcon(-1);

    HomeScreenData cached = {};
    homeScreenDataRead(&cached);
    if (cached.weather_valid) {
        showWeatherIcon(cached.weather_code);
        lv_label_set_text(label_weather_, cached.weather_line);
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
            strftime(time_buf, sizeof(time_buf), "%H:%M", &timeinfo);
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
        lv_label_set_text(label_weather_, weather.weather_line);
        showWeatherIcon(weather.weather_code);
        lv_obj_align(label_weather_, LV_ALIGN_CENTER, 0, 64);
    }
}

#include "home_view.h"

#include <time.h>

#include "../home_screen_data.h"
#include "styles.h"

void HomeView::setupView(PB_SmartKnobConfig config) {
    label_time_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_time_, &roboto_light_60, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_time_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_time_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_time_, LV_ALIGN_CENTER, 0, -42);

    label_date_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_date_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_date_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_date_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_date_, LV_ALIGN_CENTER, 0, 8);

    label_weather_icon_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_weather_icon_, &mdi_40, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_weather_icon_, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_weather_icon_, LV_ALIGN_CENTER, 0, 58);

    label_weather_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(label_weather_, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_weather_, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label_weather_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_width(label_weather_, (TFT_WIDTH * 9) / 10);
    lv_label_set_long_mode(label_weather_, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_weather_, LV_ALIGN_CENTER, 0, 98);

    lv_label_set_text(label_time_, "--:--");
    lv_label_set_text(label_date_, "Starting...");
    lv_label_set_text(label_weather_icon_, "\ue2bd");
    lv_label_set_text(label_weather_, "Loading weather");
    lv_task_handler();
}

void HomeView::updateView(PB_SmartKnobState state) {
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
            lv_label_set_text(label_date_, "Finding location...");
            homeScreenDataUpdateTime("--:--", "Finding location...", false);
        } else {
            lv_label_set_text(label_time_, "--:--");
            lv_label_set_text(label_date_, "Syncing time...");
            homeScreenDataUpdateTime("--:--", "Syncing time...", false);
        }
    }

    HomeScreenData weather = {};
    homeScreenDataRead(&weather);
    if (weather.weather_version != last_weather_version_) {
        last_weather_version_ = weather.weather_version;
        lv_label_set_text(label_weather_icon_, weather.weather_icon);
        lv_label_set_text(label_weather_, weather.weather_line);
        lv_obj_align(label_weather_icon_, LV_ALIGN_CENTER, 0, 58);
        lv_obj_align(label_weather_, LV_ALIGN_CENTER, 0, 98);
    }
}

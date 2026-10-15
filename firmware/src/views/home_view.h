#pragma once

#include "view.h"
#include "display_task.h"

class HomeView : public View {
    public:
        HomeView(lv_obj_t* screen, DisplayTask* display_task) {
            display_task_ = display_task;
            screen_ = screen;
        }
        ~HomeView() {}

        void setupView(PB_SmartKnobConfig config) override;
        void updateView(PB_SmartKnobState state) override;

    private:
        DisplayTask* display_task_;
        lv_obj_t* screen_;
        lv_obj_t* label_time_ = nullptr;
        lv_obj_t* label_date_ = nullptr;
        lv_obj_t* label_weather_icon_ = nullptr;
        lv_obj_t* label_weather_ = nullptr;
        uint32_t last_weather_version_ = 0;
        uint32_t last_time_refresh_ms_ = 0;
};

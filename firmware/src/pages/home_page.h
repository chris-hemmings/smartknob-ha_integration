#pragma once

#include "page.h"
#include "views/view.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

class HomePage : public Page {
    public:
        HomePage() : Page() {}
        ~HomePage() {}

        PB_SmartKnobConfig* getPageConfig() override;
        void handleState(PB_SmartKnobState state) override;
        void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) override;

    private:
        void openMainMenu();

        static constexpr unsigned long KNOB_WAKE_GRACE_MS = 800;
        static constexpr unsigned long KNOB_WAKE_HOLD_MS = 250;
        static constexpr float KNOB_WAKE_SUB_POSITION = 0.14f;

        unsigned long page_enter_ms_ = 0;
        unsigned long wake_movement_ms_ = 0;
        bool wake_armed_ = false;
        bool anchor_set_ = false;
        bool menu_opened_ = false;
        int32_t anchor_position_ = 0;
        float anchor_sub_position_ = 0;

        PB_SmartKnobConfig config_ = {
            .has_view_config = true,
            .view_config = {
                VIEW_HOME_SCREEN,
                "Home",
            },
            .position = 0,
            .sub_position_unit = 0,
            .position_nonce = 0,
            .min_position = 0,
            .max_position = -1,
            .infinite_scroll = false,
            .position_width_radians = 10 * PI / 180,
            .detent_strength_unit = 0.15f,
            .endstop_strength_unit = 0.2f,
            .snap_point = 0.55f,
            .detent_positions_count = 0,
            .detent_positions = {},
            .snap_point_bias = 0,
            .led_hue = 180,
        };
};

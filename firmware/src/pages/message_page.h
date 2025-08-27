#pragma once

#include "page.h"
#include "views/view.h"

class MessagePage : public Page {
    public:
        MessagePage() : Page() {}
        ~MessagePage() {}

        PB_SmartKnobConfig* getPageConfig() override;
        void handleState(PB_SmartKnobState state) override;
        void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) override;
        
        void setMessage(const char* message);
        void setCallback(std::function<void()> callback) {
            completion_callback_ = callback;
        }
        void setStrainCalibrationMode(bool active) {
            strain_calibration_active_ = active;
            strain_calibration_start_time_ = millis();
        }

    private:
        std::function<void()> completion_callback_;
        bool strain_calibration_active_ = false;
        uint32_t strain_calibration_start_time_ = 0;
        uint8_t strain_calibration_step_ = 0;
        
        PB_ViewConfig view_config = {
            VIEW_MESSAGE,
            "Message",
        };

        PB_SmartKnobConfig config_ = {
            .has_view_config = true,
            .view_config = view_config,
            .position = 0,
            .position_nonce = 1,
            .min_position = 0,
            .max_position = 0,
            .infinite_scroll = false,
            .position_width_radians = 45 * PI / 180,
            .detent_strength_unit = 0.5,
            .endstop_strength_unit = 1,
            .snap_point = 0.51,
            .led_hue = 200
        };
};

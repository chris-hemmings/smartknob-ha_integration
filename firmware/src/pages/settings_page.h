#include "page.h"
#include "interface_callbacks.h"
#include "../motor_task.h"
#include "views/view.h"
#include "message_page.h"
class MessagePage;  // Forward declaration

class SettingsPage: public Page {
    public:
        SettingsPage(MotorCalibrationCallback motor_calibration_callback, StrainCalibrationCallback strain_calibration_callback) : 
            Page(), motor_calibration_callback_(motor_calibration_callback), strain_calibration_callback_(strain_calibration_callback) {}

        ~SettingsPage(){}

        PB_SmartKnobConfig * getPageConfig() override;
        void handleState(PB_SmartKnobState state) override {};
        void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) override;
        unsigned long last_calibration_step_ = 0;
        void setMessagePage(MessagePage* message_page) { message_page_ = message_page; }

    private:
        MotorCalibrationCallback motor_calibration_callback_;
        StrainCalibrationCallback strain_calibration_callback_;
        MessagePage* message_page_ = nullptr;
        
        void handleMenuInput(int position);
        unsigned long last_interaction_time_ = 0;  // Stores last interaction timestamp
        Logger* logger_ = nullptr;


        #if SK_STRAIN
            int maxPositions = 3;
        #else
            int maxPositions = 2;
        #endif

        PB_ViewConfig view_config = {
            VIEW_LIST_MENU,
            "Settings",
            .menu_entries_count = 4,
            .menu_entries = 
            {
                {
                    "Back",
                    "\ue5c4"
                },
                {
                    "Calibrate motor",
                    ""
                },
                {
                    "Calibrate strain",
                    ""
                },
                {
                    "Restart device",
                    "\ue5d5"
                }
            }
        };

        PB_SmartKnobConfig config_ =
        {
            .has_view_config = true,
            .view_config = view_config,
            .position = 0,
            .sub_position_unit = 0,
            .position_nonce = 1,
            .min_position = 0,
            .max_position = maxPositions,
            .infinite_scroll = false,
            .position_width_radians = 45 * PI / 180,
            .detent_strength_unit = 0.5,
            .endstop_strength_unit = 1,
            .snap_point = 0.51,
            .detent_positions_count = 0,
            .detent_positions = {},
            .snap_point_bias = 0,
            .led_hue = 200
        };
};

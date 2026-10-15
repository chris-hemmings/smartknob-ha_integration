#pragma once

#include "page.h"
#include "views/view.h"
#include "../motor_task.h"

class MainMenuPage : public Page {
    public:
        MainMenuPage() : Page() {}

        ~MainMenuPage(){}

        PB_SmartKnobConfig * getPageConfig() override;
        void handleState(PB_SmartKnobState state) override;
        void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) override;
    
    private:
        void handleMenuInput(int position);
        static const unsigned long IDLE_TIMEOUT_MS = 30000;
        unsigned long last_interaction_time_ = 0;
        int32_t last_position_ = 0;

        PB_ViewConfig view_config = {
            VIEW_CIRCLE_MENU,
            "Main menu",
            .menu_entries_count = 6,
            .menu_entries = 
            {
                {
                    "Light",
                    "\ue0f0"
                },
                {
                    "Lamps",
                    "\ue2b4"
                },                
                {
                    "Fan",
                    "\uf168"
                },
                {
                    "Blinds",
                    "\uec12"
                },
                {
                    "Climate",
                    "\uf8b5"
                },
                {
                    "Settings",
                    "\ue8b8"
                },
                // {
                //     "More",
                //     "\ue5d3"
                // }
            }
        };

        PB_SmartKnobConfig config_ =
        {
            .has_view_config = true,
            .view_config = view_config,
            .position = 0,
            .sub_position_unit = 0,
            .position_nonce = 3,
            .min_position = 0,
            .max_position = 5,
            .infinite_scroll = true,
            .position_width_radians = 60 * PI / 180,
            .detent_strength_unit = 0.2,
            .endstop_strength_unit = 1,
            .snap_point = 0.5,
            .detent_positions_count = 0,
            .detent_positions = {},
            .snap_point_bias = 0,
            .led_hue = 240
        };
};
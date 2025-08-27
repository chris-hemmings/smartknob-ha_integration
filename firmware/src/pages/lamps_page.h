#include "page.h"
#include "views/view.h"
#include "../motor_task.h"
#include "../connectivity_task.h"
#include "../logger.h"
#include "secrets.h"


class LampsPage: public Page {
    public:
        LampsPage(ConnectivityTask& connectivity_task) : Page(), connectivity_task_(connectivity_task) {}

        ~LampsPage(){}

        PB_SmartKnobConfig * getPageConfig() override;
        void handleState(PB_SmartKnobState state) override;
        void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) override;

    private:
        void checkReceiveQueue();
        ConnectivityTask& connectivity_task_;
        unsigned long page_load_time_ = 0;  // Track when the page was loaded
        static const unsigned long INITIAL_DELAY_MS = 750;  // Configurable delay
        static const unsigned long PUBLISH_DELAY_MS = 1000;  // Configurable delay
        uint32_t last_publish_time;
        uint32_t last_published_position;
        Logger* logger_ = nullptr;
        unsigned long last_interaction_time_ = 0;  // Stores last interaction timestamp

        PB_SmartKnobConfig config_ =
        {
                .has_view_config = true,
                .view_config = 
                {
                    VIEW_DIAL,
                    "Lamps\nBrightness"
                },
                .position = 0,
                .sub_position_unit = 0,
                .position_nonce = 0,
                .min_position = 0,
                .max_position = 100,
                .infinite_scroll = false,
                .position_width_radians = 2.5 * PI / 180,
                .detent_strength_unit = 1,
                .endstop_strength_unit = 1,
                .snap_point = 1.5,
                .detent_positions_count = 0,
                .detent_positions = {},
                .snap_point_bias = 0,
                .led_hue = 30
        };
};

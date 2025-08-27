#include "lights_page.h"

#define TIMEOUT_MS 5000  // 5 seconds timeout

PB_SmartKnobConfig * LightsPage::getPageConfig() {
    checkReceiveQueue();
    page_load_time_ = millis();  // Record the time when page is loaded
    last_interaction_time_ = millis();  // Reset timeout on user input
    return &config_;
}

void LightsPage::checkReceiveQueue() {
    if (connectivity_task_.getLightState() != config_.position) {
        config_.position_nonce = connectivity_task_.getLightState();  // Force a UI update
        config_.position = connectivity_task_.getLightState();
        lv_task_handler();         // Process the update immediately
    }
}

void LightsPage::handleState(PB_SmartKnobState state) {
    checkReceiveQueue();

    if (last_published_position != state.current_position) {
        if (millis() - page_load_time_ >= INITIAL_DELAY_MS && millis() - last_publish_time > PUBLISH_DELAY_MS) {
            int current_position = state.current_position;
            Message msg = {
                .trigger_name = "light",
                .trigger_value = current_position
            };
            connectivity_task_.sendMqttMessage(msg);
            last_publish_time = millis();
            last_published_position = state.current_position;
        }
        last_interaction_time_ = millis();  // Reset timeout
    }

    // Check if timeout has elapsed
    if (millis() - last_interaction_time_ > TIMEOUT_MS) {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
    }    

}

void LightsPage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {    
    last_interaction_time_ = millis();  // Reset timeout on user input


    switch (input)
    {
    case INPUT_BACK:
    {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
        break;
    }
    case INPUT_FORWARD:
    {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
        break;
    }    
    default:
        break;
    }
}

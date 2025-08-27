#include "blinds_page.h"
#include <string.h>

#define TIMEOUT_MS 5000  // 5 seconds timeout

PB_SmartKnobConfig * BlindsPage::getPageConfig() {
    checkReceiveQueue();
    page_load_time_ = millis();  // Record the time when page is loaded
    last_interaction_time_ = millis();  // Reset timeout on user input
    return &config_;
}
void BlindsPage::checkReceiveQueue() {
    if (connectivity_task_.getBlindsState() != config_.position) {
        config_.position_nonce = connectivity_task_.getBlindsState();  // Force a UI update
        config_.position = connectivity_task_.getBlindsState();
        lv_task_handler();         // Process the update immediately
    }
}
void BlindsPage::handleState(PB_SmartKnobState state) {
    checkReceiveQueue();

    // Update display text based on current position
    if (state.current_position == 0) {
        strncpy(config_.view_config.description, "Blinds\nClosed", sizeof(config_.view_config.description) - 1);
        config_.view_config.description[sizeof(config_.view_config.description) - 1] = '\0';
    } else {
        strncpy(config_.view_config.description, "Blinds\nOpen", sizeof(config_.view_config.description) - 1);
        config_.view_config.description[sizeof(config_.view_config.description) - 1] = '\0';
    }

    if (last_published_position != state.current_position) {
        if (millis() - page_load_time_ >= INITIAL_DELAY_MS && millis() - last_publish_time > PUBLISH_DELAY_MS) {
            int current_position = state.current_position;
            Message msg = {
                .trigger_name = "blinds",
                .trigger_value = current_position  // Still sends 0 or 1 to MQTT
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

void BlindsPage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {   
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

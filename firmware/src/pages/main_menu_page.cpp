#include "main_menu_page.h"

PB_SmartKnobConfig * MainMenuPage::getPageConfig() {
    last_interaction_time_ = millis();
    last_position_ = config_.position;
    return &config_;
}

void MainMenuPage::handleState(PB_SmartKnobState state) {
    if (state.current_position != last_position_) {
        last_position_ = state.current_position;
        last_interaction_time_ = millis();
    }

    if (millis() - last_interaction_time_ > IDLE_TIMEOUT_MS) {
        if (page_change_callback_) {
            page_change_callback_(HOME_PAGE);
        }
    }
}

void MainMenuPage::handleMenuInput(int position) {
    last_interaction_time_ = millis();
    switch (position)
    {
    case 0:
        if (page_change_callback_) {
            page_change_callback_(LIGHTS_PAGE);
        }
        break;
    case 1:
        if (page_change_callback_) {
            page_change_callback_(LAMPS_PAGE);
        }
        break;        
    case 2:
        if (page_change_callback_) {
            page_change_callback_(FAN_PAGE);
        }
        break; 
    case 3:
        if (page_change_callback_) {
            page_change_callback_(BLINDS_PAGE);
        }
        break; 
    case 4:
        if (page_change_callback_) {
            page_change_callback_(CLIMATE_PAGE);
        }
        break;                                       
    case 5:
        if (page_change_callback_) {
            page_change_callback_(SETTINGS_PAGE);
        }
        break;
    case 6:
        if (page_change_callback_) {
            page_change_callback_(MORE_PAGE);
        }
        break;
    
    default:
        break;
    }
}

void MainMenuPage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {
    last_interaction_time_ = millis();
    switch (input)
    {
    case INPUT_FORWARD:
    {
        int current_position = state.current_position;
        handleMenuInput(current_position);
        break;
    }    
    case INPUT_WITH_DATA:
    {
        
        handleMenuInput(input_data);
        break;
    }
    default:
        break;
    }
}

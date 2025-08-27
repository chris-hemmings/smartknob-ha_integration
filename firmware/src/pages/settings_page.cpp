#include "settings_page.h"
#include "message_page.h"
#include <Arduino.h>


PB_SmartKnobConfig * SettingsPage::getPageConfig() {
    return &config_;
}



void SettingsPage::handleMenuInput(int position) {
    switch (position)
    {
    case 0:
    {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
        break;
    }
    case 1:
    {
        motor_calibration_callback_();
        break;
    }
    case 2:
    {
        // Set up message page callback to return to settings
        message_page_->setCallback([this] () {
            if (page_change_callback_) {
                page_change_callback_(SETTINGS_PAGE);
            }
        });
        
        // Set initial strain calibration message and activate mode
        message_page_->setMessage("Strain Calibration\n\nAfter 2 seconds, press and hold.");
        message_page_->setStrainCalibrationMode(true);
        
        strain_calibration_callback_();
        // Navigate to message page to show calibration progress
        if (page_change_callback_) {
            page_change_callback_(MESSAGE_PAGE);
        }
        break;
    }
    case 3:
    {
        // Restart the device
        ESP.restart();
        break;
    }
    default:
        break;
    
    }
}

void SettingsPage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {    
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

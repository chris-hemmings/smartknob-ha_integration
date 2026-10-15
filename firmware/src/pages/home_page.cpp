#include "home_page.h"

#include <math.h>

PB_SmartKnobConfig* HomePage::getPageConfig() {
    page_enter_ms_ = millis();
    wake_armed_ = false;
    config_.position = 0;
    config_.sub_position_unit = 0;
    config_.position_nonce++;
    return &config_;
}

void HomePage::handleState(PB_SmartKnobState state) {
    if (!wake_armed_) {
        if (millis() - page_enter_ms_ >= KNOB_WAKE_GRACE_MS) {
            wake_armed_ = true;
        }
        return;
    }

    if (fabsf(state.sub_position_unit) >= KNOB_WAKE_SUB_POSITION || state.current_position != 0) {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
    }
}

void HomePage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {
    if (input == INPUT_FORWARD) {
        if (page_change_callback_) {
            page_change_callback_(MAIN_MENU_PAGE);
        }
    }
}

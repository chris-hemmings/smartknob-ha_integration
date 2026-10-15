#include "home_page.h"

#include <math.h>

PB_SmartKnobConfig* HomePage::getPageConfig() {
    page_enter_ms_ = millis();
    wake_armed_ = false;
    anchor_set_ = false;
    menu_opened_ = false;
    anchor_position_ = 0;
    anchor_sub_position_ = 0;
    wake_movement_ms_ = 0;
    config_.position = 0;
    config_.sub_position_unit = 0;
    config_.position_nonce++;
    return &config_;
}

void HomePage::openMainMenu() {
    if (menu_opened_) {
        return;
    }
    menu_opened_ = true;
    if (page_change_callback_) {
        page_change_callback_(MAIN_MENU_PAGE);
    }
}

void HomePage::handleState(PB_SmartKnobState state) {
    if (!wake_armed_) {
        if (millis() - page_enter_ms_ >= KNOB_WAKE_GRACE_MS) {
            wake_armed_ = true;
            anchor_position_ = state.current_position;
            anchor_sub_position_ = state.sub_position_unit;
            anchor_set_ = true;
        }
        return;
    }

    if (!anchor_set_) {
        anchor_position_ = state.current_position;
        anchor_sub_position_ = state.sub_position_unit;
        anchor_set_ = true;
        return;
    }

    const int32_t position_delta = state.current_position - anchor_position_;
    const float sub_delta = state.sub_position_unit - anchor_sub_position_;

    if (position_delta != 0 || fabsf(sub_delta) >= KNOB_WAKE_SUB_POSITION) {
        openMainMenu();
    }
}

void HomePage::handleUserInput(input_t input, int input_data, PB_SmartKnobState state) {
    switch (input) {
    case INPUT_FORWARD:
    case INPUT_BACK:
    case INPUT_NEXT:
    case INPUT_PREV:
    case INPUT_WITH_DATA:
        openMainMenu();
        break;
    default:
        break;
    }
}

#pragma once

#include <functional>
#include <Arduino.h>
#include "../proto_gen/smartknob.pb.h"
#include "../input_type.h"

typedef enum {
    HOME_PAGE = 0,
    MAIN_MENU_PAGE,
    LIGHTS_PAGE,
    LAMPS_PAGE,
    FAN_PAGE,
    BLINDS_PAGE,
    CLIMATE_PAGE,
    SETTINGS_PAGE,
    MORE_PAGE,
    DEMO_PAGE,
    MESSAGE_PAGE
} page_t;

typedef std::function<void(page_t)> PageChangeCallback;

class Page {
    public:
        Page() {}
        virtual ~Page(){}
        virtual PB_SmartKnobConfig * getPageConfig() = 0;

        virtual void handleState(PB_SmartKnobState state) = 0;
        virtual void handleUserInput(input_t input, int input_data, PB_SmartKnobState state) = 0;
        virtual void setPageChangeCallback(PageChangeCallback cb) {
            page_change_callback_ = cb;
        }
    
    protected:
        PageChangeCallback page_change_callback_;

};

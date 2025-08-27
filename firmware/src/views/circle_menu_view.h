#pragma once

#include "view.h"
#include "display_task.h"
#include "proto_gen/smartknob.pb.h"
#include "connectivity_task.h"

class CircleMenuView: public View {
    public:
        CircleMenuView(lv_obj_t * screen, DisplayTask* display_task) {
            display_task_ = display_task;
            screen_ = screen;            
        }
        ~CircleMenuView() {}


        void setupView(PB_SmartKnobConfig config) override;
        void updateView(PB_SmartKnobState state) override;

    private:
        DisplayTask* display_task_;
        lv_obj_t * screen_;
        lv_obj_t * label_desc;
        lv_obj_t * arc;
        lv_obj_t * arc_dot;
        lv_obj_t * button1;
        lv_obj_t * button2;
        lv_obj_t * button3;
        lv_obj_t * button4;
        lv_obj_t * button5;
        lv_obj_t * button6;
        lv_obj_t * button7;
        lv_obj_t * button8;
        lv_obj_t * buttons[8] = {button1, button2, button3, button4, button5, button6, button7, button8};
        void setup_circle_menu_buttons(PB_SmartKnobConfig config);
        void setup_menu_elements();
        lv_obj_t* wifi_canvas = nullptr;
        lv_color_t* wifi_buf = nullptr;     
        lv_obj_t* mqtt_canvas = nullptr;
        lv_color_t* mqtt_buf = nullptr;
        
        // MQTT flashing variables
        unsigned long last_mqtt_flash_time = 0;
        bool mqtt_flash_state = false;
        static const unsigned long MQTT_FLASH_INTERVAL = 1000; // 1 second
        
        // WiFi flashing variables
        unsigned long last_wifi_flash_time = 0;
        bool wifi_flash_state = false;
        static const unsigned long WIFI_FLASH_INTERVAL = 1000; // 1 second
};
#pragma once

#include "lvgl.h"
#include "proto_gen/smartknob.pb.h"

const int RADIUS = TFT_WIDTH / 2;

void arc_dot_set_angle(lv_obj_t * arc_dot, float angle, int padding, int dot_size);
void set_screen_gradient(int value);
void reset_screen_background();
void setup_circle_elements(lv_obj_t ** label_desc, lv_obj_t ** arc_dot, lv_obj_t ** arc);
void setup_circle_button(lv_obj_t ** button, int index, PB_SmartKnobConfig config, lv_event_cb_t button_event_cb);
lv_obj_t* setup_wifi_canvas(lv_obj_t* parent, lv_obj_t* label_desc, lv_color_t** wifi_buf_ptr, int size = 48);
lv_obj_t* setup_mqtt_canvas(lv_obj_t* parent, lv_obj_t* label_desc, lv_color_t** mqtt_buf_ptr, int size = 48);
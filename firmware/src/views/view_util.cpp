#include <Arduino.h>
#include "view_util.h"
#include "styles.h"

void arc_dot_set_angle(lv_obj_t * arc_dot, float angle, int padding, int dot_size) {
    float reduced_radius = RADIUS - padding - dot_size / 2;
    float offset_to_center = RADIUS - dot_size / 2;
    float x = reduced_radius * sinf(angle + PI/2) + offset_to_center;
    float y = reduced_radius * cosf(angle + PI/2) + offset_to_center;
    lv_obj_set_x(arc_dot, (int16_t) x);
    lv_obj_set_y(arc_dot, (int16_t) y);
}

void set_screen_gradient(int value) {
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_grad_stop(screen, value, LV_PART_MAIN);
    lv_obj_set_style_bg_main_stop(screen, value, LV_PART_MAIN); // Sets the same value for gradient and main - gives sharp line on gradient
}

void setup_circle_elements(lv_obj_t ** label_desc, lv_obj_t ** arc_dot, lv_obj_t ** arc) {
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(screen, FILL_COLOR, LV_PART_MAIN);   
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, LV_PART_MAIN);
    set_screen_gradient(255);

    *label_desc = lv_label_create(screen);
    lv_label_set_text(*label_desc, "");
    lv_obj_set_style_text_align(*label_desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(*label_desc, LV_ALIGN_CENTER, 0, MENU_DESCRIPTION_Y_OFFSET);
    lv_obj_set_style_text_color(*label_desc, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(*label_desc, &roboto_light_24, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(*label_desc, 1, LV_PART_MAIN);

    *arc_dot = lv_obj_create(screen);
    lv_obj_set_scrollbar_mode(*arc_dot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(*arc_dot , arc_dot_size, arc_dot_size);
    arc_dot_set_angle(*arc_dot, 0, arc_dot_menu_padding, arc_dot_size);
    lv_obj_set_style_bg_color(*arc_dot , DOT_COLOR, 0);
    lv_obj_set_style_radius(*arc_dot , LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(*arc_dot, lv_color_white(), 0);
    lv_obj_set_style_border_width(*arc_dot, 2, 0);

    *arc = lv_arc_create(screen);
    lv_obj_set_size(*arc, TFT_WIDTH, TFT_HEIGHT);
    lv_obj_align(*arc,LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(*arc, 8, LV_PART_MAIN);
    lv_arc_set_bg_angles(*arc, 0, 0); // Hides arc background by setting background arc angles to 0
    lv_obj_remove_style(*arc, NULL, LV_PART_KNOB); // Hides arc knob - for arc on the boundaries we only want the indicator part of the lvgl arc widget
    lv_obj_set_style_arc_color(*arc, DOT_COLOR, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(*arc, 5, LV_PART_INDICATOR);
    lv_arc_set_mode(*arc, LV_ARC_MODE_REVERSE);
}

void setup_circle_button(lv_obj_t ** button, int index, PB_SmartKnobConfig config, lv_event_cb_t button_event_cb) {
    lv_obj_t * screen = lv_scr_act();
    float reduced_radius = RADIUS - button_padding - button_size / 2;
    float offset_to_center = RADIUS - button_size / 2;
    // 𝑥=𝑟∗𝑠𝑖𝑛(θ),𝑦=𝑟∗𝑐𝑜𝑠(θ)
    float x = reduced_radius * sinf(-index * config.position_width_radians + PI) + offset_to_center;
    float y = reduced_radius * cosf(-index * config.position_width_radians + PI) + offset_to_center;
    *button = lv_btn_create(screen);

    lv_obj_t * label = lv_label_create(*button);

    lv_label_set_text(label, config.view_config.menu_entries[index].icon);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label, &mdi_40, LV_PART_MAIN);

    lv_obj_set_size(*button, button_size, button_size);
    lv_obj_set_x(*button, (int16_t) x);
    lv_obj_set_y(*button, (int16_t) y);
    lv_obj_add_style(*button, &circle_btn_style, 0);
    lv_obj_add_style(*button, &circle_btn_highlighted_style, LV_STATE_FOCUSED);

    lv_obj_set_user_data(*button, (void*) index);
    lv_obj_add_event_cb(*button, button_event_cb, LV_EVENT_PRESSED, NULL);
    
    // Hide button if menu entry is empty (no description or icon)
    if (strlen(config.view_config.menu_entries[index].description) == 0 || 
        strlen(config.view_config.menu_entries[index].icon) == 0) {
        lv_obj_add_flag(*button, LV_OBJ_FLAG_HIDDEN);
    }
}

// view_util.cpp
lv_obj_t* setup_wifi_canvas(lv_obj_t* parent, lv_obj_t* label_desc, lv_color_t** wifi_buf_ptr, int size) {
    *wifi_buf_ptr = (lv_color_t*)heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(size, size) * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    lv_obj_t* wifi_canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(wifi_canvas, *wifi_buf_ptr, size, size, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align_to(wifi_canvas, label_desc, LV_ALIGN_OUT_BOTTOM_MID, -13, 4);
    lv_obj_move_background(wifi_canvas);
    lv_obj_add_flag(wifi_canvas, LV_OBJ_FLAG_HIDDEN);
    return wifi_canvas;
}

// view_util.cpp
lv_obj_t* setup_mqtt_canvas(lv_obj_t* parent, lv_obj_t* label_desc, lv_color_t** mqtt_buf_ptr, int size) {
    *mqtt_buf_ptr = (lv_color_t*)heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(size, size) * sizeof(lv_color_t), MALLOC_CAP_8BIT);
    lv_obj_t* mqtt_canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(mqtt_canvas, *mqtt_buf_ptr, size, size, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align_to(mqtt_canvas, label_desc, LV_ALIGN_OUT_BOTTOM_MID, 13, 4);
    lv_obj_move_background(mqtt_canvas);
    lv_obj_add_flag(mqtt_canvas, LV_OBJ_FLAG_HIDDEN);
    return mqtt_canvas;
} 

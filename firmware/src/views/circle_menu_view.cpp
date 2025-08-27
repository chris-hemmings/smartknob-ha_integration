#include <Arduino.h>

#include "circle_menu_view.h"
#include "view_util.h"
#include "styles.h"
#include <WiFi.h>

void CircleMenuView::setup_circle_menu_buttons(PB_SmartKnobConfig config) {
  for (int i=0; i < config.max_position + 1; i++) {
    setup_circle_button(&buttons[i], i, config, display_task_->button_event_cb);
  }
}

void CircleMenuView::setup_menu_elements() {
  lv_obj_align(label_desc, LV_ALIGN_CENTER, 0, MENU_DESCRIPTION_Y_OFFSET);
}

void CircleMenuView::setupView(PB_SmartKnobConfig config) {
  // setup_circle_elements(config);
  setup_circle_elements(&label_desc, &arc_dot, &arc);
  setup_circle_menu_buttons(config);
  setup_menu_elements();  
  wifi_canvas = setup_wifi_canvas(lv_obj_get_parent(label_desc), label_desc, &wifi_buf, 24);
  mqtt_canvas = setup_mqtt_canvas(lv_obj_get_parent(label_desc), label_desc, &mqtt_buf, 24);
}

void CircleMenuView::updateView(PB_SmartKnobState state) {



  if (WiFi.status() == WL_CONNECTED) {
      lv_canvas_fill_bg(wifi_canvas, lv_color_black(), LV_OPA_TRANSP);

      lv_draw_arc_dsc_t arc_dsc;
      lv_draw_arc_dsc_init(&arc_dsc);
      arc_dsc.color = lv_color_white();
      arc_dsc.width = 2;

      // Use 24x24 canvas coordinates and smaller radii
      lv_canvas_draw_arc(wifi_canvas, 12, 16, 10, 210, 330, &arc_dsc);  // 30° span
      lv_canvas_draw_arc(wifi_canvas, 12, 16, 7, 210, 330, &arc_dsc);   // 30° span
      lv_canvas_draw_arc(wifi_canvas, 12, 16, 4, 210, 330, &arc_dsc);   // 30° span

      lv_draw_rect_dsc_t rect_dsc;
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.bg_color = lv_color_white();
      rect_dsc.radius = LV_RADIUS_CIRCLE;
      lv_canvas_draw_rect(wifi_canvas, 10, 16, 4, 4, &rect_dsc);  // Centered on arcs

      lv_obj_clear_flag(wifi_canvas, LV_OBJ_FLAG_HIDDEN);
  } else {
      // WiFi flashing logic when not connected
      unsigned long current_time = millis();
      
      // Check if it's time to toggle the flash state
      if (current_time - last_wifi_flash_time >= WIFI_FLASH_INTERVAL) {
          wifi_flash_state = !wifi_flash_state;
          last_wifi_flash_time = current_time;
      }
      
      // Only show the WiFi canvas when flash state is true (flashing on)
      if (wifi_flash_state) {
          lv_canvas_fill_bg(wifi_canvas, lv_color_black(), LV_OPA_TRANSP);

          lv_draw_arc_dsc_t arc_dsc;
          lv_draw_arc_dsc_init(&arc_dsc);
          arc_dsc.color = lv_color_white();
          arc_dsc.width = 2;

          // Use 24x24 canvas coordinates and smaller radii
          lv_canvas_draw_arc(wifi_canvas, 12, 16, 10, 210, 330, &arc_dsc);  // 30° span
          lv_canvas_draw_arc(wifi_canvas, 12, 16, 7, 210, 330, &arc_dsc);   // 30° span
          lv_canvas_draw_arc(wifi_canvas, 12, 16, 4, 210, 330, &arc_dsc);   // 30° span

          lv_draw_rect_dsc_t rect_dsc;
          lv_draw_rect_dsc_init(&rect_dsc);
          rect_dsc.bg_color = lv_color_white();
          rect_dsc.radius = LV_RADIUS_CIRCLE;
          lv_canvas_draw_rect(wifi_canvas, 10, 16, 4, 4, &rect_dsc);  // Centered on arcs

          lv_obj_clear_flag(wifi_canvas, LV_OBJ_FLAG_HIDDEN);
      } else {
          lv_obj_add_flag(wifi_canvas, LV_OBJ_FLAG_HIDDEN);
      }
  }

  // MQTT connection status display

  // Check MQTT connection status using the static instance
  bool mqtt_connected = false;
  if (ConnectivityTask::instance != nullptr) {
      mqtt_connected = ConnectivityTask::instance->isMqttConnected();
  }

  if (mqtt_connected) {
      lv_canvas_fill_bg(mqtt_canvas, lv_color_black(), LV_OPA_TRANSP);

      lv_draw_rect_dsc_t circle_dsc;
      lv_draw_rect_dsc_init(&circle_dsc);
      circle_dsc.bg_color = lv_color_white();
      circle_dsc.radius = LV_RADIUS_CIRCLE;
      
      // Draw three overlapping circles to form a cloud shape
      lv_canvas_draw_rect(mqtt_canvas, 6, 10, 6, 6, &circle_dsc);   // left puff
      lv_canvas_draw_rect(mqtt_canvas, 10, 6, 8, 8, &circle_dsc);   // center puff (larger)
      lv_canvas_draw_rect(mqtt_canvas, 16, 10, 6, 6, &circle_dsc);  // right puff
      
      // Optional: add a base rectangle to flatten the bottom
      lv_canvas_draw_rect(mqtt_canvas, 8, 12, 12, 4, &circle_dsc);  // cloud base
      
      lv_obj_clear_flag(mqtt_canvas, LV_OBJ_FLAG_HIDDEN);
  } else {
      // MQTT flashing logic when not connected
      unsigned long current_time = millis();
      
      // Check if it's time to toggle the flash state
      if (current_time - last_mqtt_flash_time >= MQTT_FLASH_INTERVAL) {
          mqtt_flash_state = !mqtt_flash_state;
          last_mqtt_flash_time = current_time;
      }
      
      // Only show the MQTT canvas when flash state is true (flashing on)
      if (mqtt_flash_state) {
          lv_canvas_fill_bg(mqtt_canvas, lv_color_black(), LV_OPA_TRANSP);

          lv_draw_rect_dsc_t circle_dsc;
          lv_draw_rect_dsc_init(&circle_dsc);
          circle_dsc.bg_color = lv_color_white();
          circle_dsc.radius = LV_RADIUS_CIRCLE;
          
          // Draw three overlapping circles to form a cloud shape
          lv_canvas_draw_rect(mqtt_canvas, 6, 10, 6, 6, &circle_dsc);   // left puff
          lv_canvas_draw_rect(mqtt_canvas, 10, 6, 8, 8, &circle_dsc);   // center puff (larger)
          lv_canvas_draw_rect(mqtt_canvas, 16, 10, 6, 6, &circle_dsc);  // right puff
          
          // Optional: add a base rectangle to flatten the bottom
          lv_canvas_draw_rect(mqtt_canvas, 8, 12, 12, 4, &circle_dsc);  // cloud base
          
          lv_obj_clear_flag(mqtt_canvas, LV_OBJ_FLAG_HIDDEN);
      } else {
          lv_obj_add_flag(mqtt_canvas, LV_OBJ_FLAG_HIDDEN);
      }
  }
 
  if (lv_obj_is_valid(buttons[0]) && state.current_position >= 0 && state.current_position <= state.config.max_position) {
    for (int i=0; i < state.config.max_position + 1; i++) {
      lv_obj_clear_state(buttons[i], LV_STATE_FOCUSED);
    }
    lv_obj_add_state(buttons[state.current_position], LV_STATE_FOCUSED);
  }

  float left_bound = PI / 2;
  float right_bound = PI / 2;
  float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;

  int32_t num_positions = state.config.max_position - state.config.min_position + 1;
  lv_label_set_text(label_desc, state.config.view_config.menu_entries[state.current_position].description);
  lv_obj_set_style_text_align(label_desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(label_desc, LV_ALIGN_CENTER, 0, MENU_DESCRIPTION_Y_OFFSET); // Realigns label and its content to the center - this is required every time after setting text value
  lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);

  float raw_angle = left_bound - (state.current_position - state.config.min_position) * state.config.position_width_radians;
  float adjusted_angle = raw_angle - adjusted_sub_position;

  if (num_positions > 0 && state.config.infinite_scroll == 0 && ((state.current_position == state.config.min_position && state.sub_position_unit < 0) || (state.current_position == state.config.max_position && state.sub_position_unit > 0))) {
    arc_dot_set_angle(arc_dot, raw_angle, arc_dot_menu_padding, arc_dot_size);
  } else {
    arc_dot_set_angle(arc_dot, adjusted_angle, arc_dot_menu_padding, arc_dot_size);
  }
}
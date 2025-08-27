#include "dial_view.h"
#include "styles.h"
#include "view_util.h"

void DialView::setup_dial_elements(PB_SmartKnobConfig config) {
  line_left_bound = lv_line_create(screen_);
  lv_obj_add_style(line_left_bound,&style_line, 0);
  lv_obj_set_pos(line_left_bound, 0, 0);

  line_right_bound = lv_line_create(screen_);
  lv_obj_add_style(line_right_bound,&style_line, 0);
  lv_obj_set_pos(line_right_bound, 0, 0);

  label_cur_pos = lv_label_create(screen_);
  lv_obj_set_style_text_align(label_cur_pos, LV_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(label_cur_pos, LV_ALIGN_CENTER, 0, -25);
  lv_label_set_text(label_cur_pos, "0");
  lv_obj_set_style_text_color(label_cur_pos, lv_color_white(), 0);
  lv_obj_set_style_text_font(label_cur_pos, &roboto_light_60, 0);

  lv_obj_align(label_desc, LV_ALIGN_CENTER, 0, DIAL_DESCRIPTION_Y_OFFSET);
}

void DialView::setupView(PB_SmartKnobConfig config) {
    setup_circle_elements(&label_desc, &arc_dot, &arc);
    setup_dial_elements(config);
}

void DialView::updateView(PB_SmartKnobState state) {
    float left_bound = PI / 2;
    float right_bound = PI / 2;
    float adjusted_sub_position = state.sub_position_unit * state.config.position_width_radians;

    int32_t num_positions = state.config.max_position - state.config.min_position + 1;

    lv_label_set_text(label_desc, state.config.view_config.description);
    lv_obj_set_style_text_align(label_desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_desc, LV_ALIGN_CENTER, 0, DIAL_DESCRIPTION_Y_OFFSET); // Realigns label and its content to the center - this is required every time after setting text value 

    if (num_positions > 1) {
    int32_t fill_height = 255 - state.current_position * 255 / (num_positions - 1);
        set_screen_gradient(fill_height);
    } else {
        set_screen_gradient(255);
    }

    // Check if this is the blinds page and show custom text
    if (strstr(state.config.view_config.description, "Blinds") != nullptr) {
        if (state.current_position == 0) {
            lv_label_set_text(label_cur_pos, "Closed");
        } else {
            lv_label_set_text(label_cur_pos, "Open");
        }
    } else if (strstr(state.config.view_config.description, "Air Con") != nullptr) {
        if (state.current_position == 0) {
            lv_label_set_text(label_cur_pos, "Off");
        } else {
            lv_label_set_text(label_cur_pos, "On");
        }
    } else {
        // Default numeric display for other pages
        lv_label_set_text_fmt(label_cur_pos, "%d", state.current_position);
    }
    lv_obj_set_style_text_align(label_cur_pos, LV_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label_cur_pos, LV_ALIGN_CENTER, 0, -25);

    if (num_positions > 0 && state.config.infinite_scroll == 0) {
    if (state.current_position == state.config.min_position && state.sub_position_unit < 0) {
        adjusted_sub_position = -logf(1 - state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180) * 5 * PI / 180;
    } else if (state.current_position == state.config.max_position && state.sub_position_unit > 0) {
        adjusted_sub_position = logf(1 + state.sub_position_unit  * state.config.position_width_radians / 5 / PI * 180)  * 5 * PI / 180;
    }
    }

    if (num_positions > 0 && state.config.infinite_scroll == 0) {
    float range_radians = (state.config.max_position - state.config.min_position) * state.config.position_width_radians;
    left_bound = PI / 2 + range_radians / 2;
    right_bound = PI / 2 - range_radians / 2;
    points_left_bound[0] = {
                            (lv_coord_t)(TFT_WIDTH/2 + RADIUS * cosf(left_bound)),
                            (lv_coord_t)(TFT_HEIGHT/2 - RADIUS * sinf(left_bound))
                            };
    points_left_bound[1] = {
                            (lv_coord_t)(TFT_WIDTH/2 + (RADIUS - 10) * cosf(left_bound)),
                            (lv_coord_t)(TFT_HEIGHT/2 - (RADIUS - 10) * sinf(left_bound))
                            };
    lv_line_set_points(line_left_bound,points_left_bound,2);
    points_right_bound[0] = {
                            (lv_coord_t)(TFT_WIDTH/2 + RADIUS * cosf(right_bound)),
                            (lv_coord_t)(TFT_HEIGHT/2 - RADIUS * sinf(right_bound))
                            };
    points_right_bound[1] = {
                            (lv_coord_t)(TFT_WIDTH/2 + (RADIUS - 10) * cosf(right_bound)),
                            (lv_coord_t)(TFT_HEIGHT/2 - (RADIUS - 10) * sinf(right_bound))
                            };                           
    lv_line_set_points(line_right_bound, points_right_bound,2);
    }
    else {
    points_left_bound[0] = {0,0};
    points_left_bound[1] = {0,0};
    lv_line_set_points(line_left_bound, points_left_bound, 2);
    points_right_bound[0] = {0,0};
    points_right_bound[1] = {0,0};
    lv_line_set_points(line_right_bound, points_right_bound, 2);
    }

    float raw_angle = left_bound - (state.current_position - state.config.min_position) * state.config.position_width_radians;
    float adjusted_angle = raw_angle - adjusted_sub_position;

    int32_t raw_angle_offset = (int) (-((raw_angle * (180/PI)) - 90));
    int32_t adjusted_angle_offset = (int) (-((adjusted_angle * (180/PI)) - 90));

    if (num_positions > 0 && state.config.infinite_scroll == 0 && ((state.current_position == state.config.min_position && state.sub_position_unit < 0) || (state.current_position == state.config.max_position && state.sub_position_unit > 0))) {
    arc_dot_set_angle(arc_dot, raw_angle, arc_dot_dial_padding, arc_dot_size);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_HIDDEN);
        if (raw_angle_offset < adjusted_angle_offset) {
            lv_arc_set_rotation(arc, 270 + left_bound * (180/PI));
            lv_arc_set_angles(arc, 270, 270 - (raw_angle_offset-adjusted_angle_offset));
        } else {
            lv_arc_set_rotation(arc, 270 + right_bound * (180/PI));
            lv_arc_set_angles(arc, 270 - (raw_angle_offset-adjusted_angle_offset), 270);
        }
    } else {
        lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
        arc_dot_set_angle(arc_dot, adjusted_angle, arc_dot_dial_padding, arc_dot_size);
    }
}
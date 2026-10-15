#include "weather_icon_draw.h"

#include <math.h>

static const int ICON_CANVAS_SIZE = 52;

static lv_color_t color_hex(uint32_t rgb) {
    return lv_color_make((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

static void draw_disc(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy, lv_coord_t r, lv_color_t color) {
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.radius = LV_RADIUS_CIRCLE;
    dsc.border_width = 0;
    lv_canvas_draw_rect(canvas, cx - r, cy - r, r * 2, r * 2, &dsc);
}

static void draw_cloud_blob(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy, lv_color_t color) {
    draw_disc(canvas, cx - 10, cy + 2, 9, color);
    draw_disc(canvas, cx, cy - 4, 11, color);
    draw_disc(canvas, cx + 11, cy + 1, 10, color);
    draw_disc(canvas, cx + 2, cy + 5, 12, color);
}

static void draw_sun(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy, lv_coord_t r) {
    const lv_color_t sun = color_hex(0xFFCA28);
    draw_disc(canvas, cx, cy, r, sun);

    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = sun;
    line.width = 3;
    line.round_start = 1;
    line.round_end = 1;

    for (int i = 0; i < 8; i++) {
        const float a = static_cast<float>(i) * 3.14159265f / 4.0f;
        const lv_point_t pts[2] = {
            {static_cast<lv_coord_t>(cx + (r + 3) * cosf(a)),
             static_cast<lv_coord_t>(cy + (r + 3) * sinf(a))},
            {static_cast<lv_coord_t>(cx + (r + 9) * cosf(a)),
             static_cast<lv_coord_t>(cy + (r + 9) * sinf(a))},
        };
        lv_canvas_draw_line(canvas, pts, 2, &line);
    }
}

static void draw_rain(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy) {
    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = color_hex(0x42A5F5);
    line.width = 2;
    line.round_start = 1;
    line.round_end = 1;
    for (int i = -1; i <= 1; i++) {
        const lv_point_t pts[2] = {
            {static_cast<lv_coord_t>(cx + i * 8 - 2), static_cast<lv_coord_t>(cy + 10)},
            {static_cast<lv_coord_t>(cx + i * 8 - 6), static_cast<lv_coord_t>(cy + 18)},
        };
        lv_canvas_draw_line(canvas, pts, 2, &line);
    }
}

static void draw_snow(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy) {
    const lv_color_t snow = color_hex(0xE3F2FD);
    draw_disc(canvas, cx - 8, cy + 14, 2, snow);
    draw_disc(canvas, cx, cy + 17, 2, snow);
    draw_disc(canvas, cx + 9, cy + 15, 2, snow);
}

static void draw_lightning(lv_obj_t* canvas, lv_coord_t cx, lv_coord_t cy) {
    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = color_hex(0xFFEB3B);
    line.width = 3;
    line.round_start = 0;
    line.round_end = 0;
    const lv_point_t pts[4] = {
        {cx + 2, cy + 6},
        {cx - 4, cy + 16},
        {cx + 3, cy + 16},
        {cx - 2, cy + 26},
    };
    lv_canvas_draw_line(canvas, pts, 2, &line);
    lv_canvas_draw_line(canvas, &pts[1], 2, &line);
}

void weather_icon_draw(lv_obj_t* canvas, int wmo_code) {
    if (canvas == nullptr) {
        return;
    }

    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_TRANSP);

    const lv_coord_t cx = ICON_CANVAS_SIZE / 2;
    const lv_coord_t cy = ICON_CANVAS_SIZE / 2 - 2;
    const lv_color_t cloud = color_hex(0xCFD8DC);
    const lv_color_t cloud_dark = color_hex(0x90A4AE);

    if (wmo_code == 0) {
        draw_sun(canvas, cx, cy, 11);
        return;
    }

    if (wmo_code == 1 || wmo_code == 2) {
        draw_sun(canvas, cx - 10, cy - 2, 8);
        draw_cloud_blob(canvas, cx + 6, cy + 4, cloud);
        return;
    }

    if (wmo_code == 3) {
        draw_cloud_blob(canvas, cx, cy, cloud);
        return;
    }

    if (wmo_code == 45 || wmo_code == 48) {
        draw_cloud_blob(canvas, cx, cy - 2, cloud_dark);
        lv_draw_line_dsc_t line;
        lv_draw_line_dsc_init(&line);
        line.color = color_hex(0xB0BEC5);
        line.width = 2;
        for (int i = -1; i <= 1; i++) {
            const lv_point_t pts[2] = {{cx + i * 10 - 8, cy + 14}, {cx + i * 10 + 8, cy + 14}};
            lv_canvas_draw_line(canvas, pts, 2, &line);
        }
        return;
    }

    if ((wmo_code >= 51 && wmo_code <= 67) || (wmo_code >= 80 && wmo_code <= 82)) {
        draw_cloud_blob(canvas, cx, cy - 4, cloud);
        draw_rain(canvas, cx, cy);
        return;
    }

    if ((wmo_code >= 71 && wmo_code <= 77) || (wmo_code >= 85 && wmo_code <= 86)) {
        draw_cloud_blob(canvas, cx, cy - 4, color_hex(0xB3E5FC));
        draw_snow(canvas, cx, cy);
        return;
    }

    if (wmo_code >= 95) {
        draw_cloud_blob(canvas, cx, cy - 6, cloud_dark);
        draw_lightning(canvas, cx + 2, cy);
        return;
    }

    draw_cloud_blob(canvas, cx, cy, cloud);
}

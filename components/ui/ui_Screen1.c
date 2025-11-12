#include "ui.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include <stdio.h>
#include <math.h>
#include "esp_log.h"

lv_obj_t * ui_Screen1 = NULL;
lv_obj_t * ui_SpeakButton = NULL;
lv_obj_t * ui_SpeakButtonLabel = NULL;
lv_obj_t * ui_TimeLabel = NULL;
lv_obj_t * ui_DateLabel = NULL;
lv_obj_t * ui_StatusLabel = NULL;
lv_obj_t * ui_FormatSwitch = NULL;
lv_obj_t * ui_Format24Label = NULL;
lv_obj_t * ui_Format12Label = NULL;
lv_obj_t * ui_AnalogClock = NULL;
lv_obj_t * ui_hour_hand = NULL;
lv_obj_t * ui_minute_hand = NULL;
lv_obj_t * ui_second_hand = NULL;
lv_obj_t * ui_center_dot = NULL;
lv_obj_t * clock_face = NULL;
lv_obj_t * ui_SpeakDateButton = NULL;
lv_obj_t * ui_SpeakDateButtonLabel = NULL;
lv_obj_t * ui_LeftNavBtn_S1 = NULL;
lv_obj_t * ui_RightNavBtn_S1 = NULL;
static lv_obj_t * ui_NavDots_S1 = NULL;
static lv_obj_t * ui_NavDot1_S1 = NULL;
static lv_obj_t * ui_NavDot2_S1 = NULL;
static lv_obj_t * ui_NavDot3_S1 = NULL;
static lv_obj_t * ui_NavDot4_S1 = NULL;
static lv_point_t hour_points[2];
static lv_point_t minute_points[2];
static lv_point_t second_points[2];

static const char *TAG_CLOCK = "ANALOG_CLOCK";

void ui_event_Screen1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;
        
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(indev);
        
        if(gesture_dir == LV_DIR_LEFT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
        }
        else if(gesture_dir == LV_DIR_RIGHT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen4_screen_init);
        }
    }
}

void ui_event_LeftNavBtn_S1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_CLOCK, "Left navigation button pressed - going to Screen4");
        _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen4_screen_init);
    }
}

void ui_event_RightNavBtn_S1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_CLOCK, "Right navigation button pressed - going to Screen2");
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
    }
}

static void create_navigation_buttons(lv_obj_t * parent)
{
    ui_LeftNavBtn_S1 = lv_btn_create(parent);
    lv_obj_set_size(ui_LeftNavBtn_S1, 50, 50); 
    lv_obj_align(ui_LeftNavBtn_S1, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S1, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S1, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_LeftNavBtn_S1, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_LeftNavBtn_S1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_LeftNavBtn_S1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_LeftNavBtn_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t * left_arrow = lv_label_create(ui_LeftNavBtn_S1);
    lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(left_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(left_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(left_arrow);
    lv_obj_set_style_pad_all(left_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(left_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_LeftNavBtn_S1, ui_event_LeftNavBtn_S1, LV_EVENT_CLICKED, NULL);
    
    ui_RightNavBtn_S1 = lv_btn_create(parent);
    lv_obj_set_size(ui_RightNavBtn_S1, 50, 50);
    lv_obj_align(ui_RightNavBtn_S1, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S1, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S1, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_RightNavBtn_S1, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_RightNavBtn_S1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_RightNavBtn_S1, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_RightNavBtn_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT); 
    
    lv_obj_t * right_arrow = lv_label_create(ui_RightNavBtn_S1);
    lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(right_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(right_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(right_arrow);
    lv_obj_set_style_pad_all(right_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(right_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_RightNavBtn_S1, ui_event_RightNavBtn_S1, LV_EVENT_CLICKED, NULL);
    
    
}

static void calculate_hand_points_in_box(float angle_deg, int length, int box_w, int box_h, lv_point_t *out_points)
{
    float angle_rad = (angle_deg - 90.0f) * M_PI / 180.0f;

    int cx = box_w / 2;
    int cy = box_h / 2;

    int x = (int)(length * cosf(angle_rad));
    int y = (int)(length * sinf(angle_rad));

    out_points[0].x = cx;
    out_points[0].y = cy;

    out_points[1].x = cx + x;
    out_points[1].y = cy + y;

    ESP_LOGD(TAG_CLOCK, "calc: ang=%.1f len=%d box=(%d,%d) -> p0=(%d,%d) p1=(%d,%d)",
             angle_deg, length, box_w, box_h,
             out_points[0].x, out_points[0].y,
             out_points[1].x, out_points[1].y);
}

static void create_analog_clock(lv_obj_t * parent)
{
    ESP_LOGI(TAG_CLOCK, "Creating analog clock...");
    ui_AnalogClock = lv_obj_create(parent);
    lv_obj_set_size(ui_AnalogClock, 200, 200);
    lv_obj_align(ui_AnalogClock, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_bg_color(ui_AnalogClock, lv_color_hex(0x2D2D2D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_AnalogClock, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_AnalogClock, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_AnalogClock, LV_OBJ_FLAG_SCROLLABLE);

    clock_face = lv_obj_create(ui_AnalogClock);
    lv_obj_set_size(clock_face, 180, 180);
    lv_obj_center(clock_face);
    lv_obj_set_style_bg_color(clock_face, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(clock_face, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(clock_face, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(clock_face, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(clock_face, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(clock_face, LV_OBJ_FLAG_SCROLLABLE);

    const char *hour_labels[12] = {"12","1","2","3","4","5","6","7","8","9","10","11"};
    const int clock_radius = 70; 

    for (int i = 0; i < 12; ++i) {
        float angle_deg = i * 30.0f;
        float angle_rad = (angle_deg - 90.0f) * M_PI / 180.0f;
        int x = (int)(clock_radius * cosf(angle_rad));
        int y = (int)(clock_radius * sinf(angle_rad));

        lv_obj_t *label = lv_label_create(clock_face);
        lv_label_set_text(label, hour_labels[i]);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_align(label, LV_ALIGN_CENTER, x, y);
    }

    const int face_w = 180;
    const int face_h = 180;

    ui_hour_hand = lv_line_create(clock_face);
    lv_line_set_points(ui_hour_hand, hour_points, 2);
    lv_obj_set_size(ui_hour_hand, face_w, face_h);
    lv_obj_align(ui_hour_hand, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_line_width(ui_hour_hand, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui_hour_hand, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui_hour_hand, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_minute_hand = lv_line_create(clock_face);
    lv_line_set_points(ui_minute_hand, minute_points, 2);
    lv_obj_set_size(ui_minute_hand, face_w, face_h);
    lv_obj_align(ui_minute_hand, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_line_width(ui_minute_hand, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui_minute_hand, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui_minute_hand, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_second_hand = lv_line_create(clock_face);
    lv_line_set_points(ui_second_hand, second_points, 2);
    lv_obj_set_size(ui_second_hand, face_w, face_h);
    lv_obj_align(ui_second_hand, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_line_width(ui_second_hand, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui_second_hand, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui_second_hand, true, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_center_dot = lv_obj_create(clock_face);
    lv_obj_set_size(ui_center_dot, 10, 10);
    lv_obj_center(ui_center_dot);
    lv_obj_set_style_bg_color(ui_center_dot, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_center_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_center_dot, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_move_foreground(ui_second_hand);
    lv_obj_move_foreground(ui_center_dot);

    ESP_LOGI(TAG_CLOCK, "Analog clock created (face %dx%d)", face_w, face_h);
}

static void create_navigation_dots(lv_obj_t * parent)
{
    ui_NavDots_S1 = lv_obj_create(parent);
    lv_obj_set_size(ui_NavDots_S1, 100, 20);
    lv_obj_align(ui_NavDots_S1, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(ui_NavDots_S1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDots_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_NavDots_S1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ui_NavDots_S1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_NavDots_S1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(ui_NavDots_S1, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot1_S1 = lv_obj_create(ui_NavDots_S1);
    lv_obj_set_size(ui_NavDot1_S1, 10, 10);
    lv_obj_set_style_bg_color(ui_NavDot1_S1, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot1_S1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot1_S1, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot1_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot2_S1 = lv_obj_create(ui_NavDots_S1);
    lv_obj_set_size(ui_NavDot2_S1, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot2_S1, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot2_S1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot2_S1, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot2_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot3_S1 = lv_obj_create(ui_NavDots_S1);
    lv_obj_set_size(ui_NavDot3_S1, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot3_S1, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot3_S1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot3_S1, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot3_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot4_S1 = lv_obj_create(ui_NavDots_S1);
    lv_obj_set_size(ui_NavDot4_S1, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot4_S1, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot4_S1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot4_S1, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot4_S1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void update_analog_clock(int hours, int minutes, int seconds)
{
    if (!clock_face || !ui_hour_hand || !ui_minute_hand || !ui_second_hand) {
        ESP_LOGE(TAG_CLOCK, "Analog clock not initialized!");
        return;
    }
    int face_w = lv_obj_get_width(clock_face);
    int face_h = lv_obj_get_height(clock_face);

    float second_angle = seconds * 6.0f; 
    float minute_angle = minutes * 6.0f + seconds * 0.1f; 
    float hour_angle = (hours % 12) * 30.0f + minutes * 0.5f;

    ESP_LOGD(TAG_CLOCK, "UPDATE: %02d:%02d:%02d -> H:%.1f M:%.1f S:%.1f",
             hours, minutes, seconds, hour_angle, minute_angle, second_angle);

    calculate_hand_points_in_box(hour_angle, 30, face_w, face_h, hour_points);
    lv_line_set_points(ui_hour_hand, hour_points, 2);

    calculate_hand_points_in_box(minute_angle, 45, face_w, face_h, minute_points);
    lv_line_set_points(ui_minute_hand, minute_points, 2);

    calculate_hand_points_in_box(second_angle, 60, face_w, face_h, second_points);
    lv_line_set_points(ui_second_hand, second_points, 2);

    lv_obj_invalidate(clock_face);
}

void ui_Screen1_screen_init(void)
{
    ESP_LOGI(TAG_CLOCK, "Initializing Screen1 (clean layout)...");

    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x0D0D0D), 0);
    lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, 0);

    lv_obj_t * top_panel = lv_obj_create(ui_Screen1);
    lv_obj_set_size(top_panel, lv_pct(100), 50);
    lv_obj_align(top_panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_panel, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(top_panel, 0, 0);
    lv_obj_clear_flag(top_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(top_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_panel,
        LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(top_panel, 30, 0);
    lv_obj_set_style_pad_ver(top_panel, 8, 0);


    ui_DateLabel = lv_label_create(top_panel);
    lv_label_set_text(ui_DateLabel, "2025-01-01");
    lv_obj_set_style_text_color(ui_DateLabel, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(ui_DateLabel, &lv_font_montserrat_20, 0);

    lv_obj_t * title_label = lv_label_create(top_panel);
    lv_label_set_text(title_label, "Clock station");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_center(title_label);

    ui_StatusLabel = lv_label_create(top_panel);
    lv_label_set_text(ui_StatusLabel, "System starting...");
    lv_obj_set_style_text_color(ui_StatusLabel, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_StatusLabel, &lv_font_montserrat_12, 0);

    create_analog_clock(ui_Screen1);
    lv_obj_align(ui_AnalogClock, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t * bottom_area = lv_obj_create(ui_Screen1);
    lv_obj_set_size(bottom_area, lv_pct(90), 180);
    lv_obj_align(bottom_area, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(bottom_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(bottom_area, 0, 0);
    lv_obj_clear_flag(bottom_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(bottom_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(bottom_area,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(bottom_area, 0, 0);
    lv_obj_set_style_pad_row(bottom_area, 15, 0);

    lv_obj_t * time_format_row = lv_obj_create(bottom_area);
    lv_obj_set_size(time_format_row, 450, 60); 
    lv_obj_set_style_bg_opa(time_format_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_format_row, 0, 0);
    lv_obj_clear_flag(time_format_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(time_format_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(time_format_row,
        LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); 
    lv_obj_set_style_pad_all(time_format_row, 0, 0); 

    ui_TimeLabel = lv_label_create(time_format_row);
    lv_label_set_text(ui_TimeLabel, "12:00:00");
    lv_obj_set_style_text_color(ui_TimeLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_TimeLabel, &lv_font_montserrat_30, 0);
    lv_obj_set_width(ui_TimeLabel, 150);
    lv_obj_set_style_text_align(ui_TimeLabel, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t * format_container = lv_obj_create(time_format_row);
    lv_obj_set_size(format_container, 160, 40);
    lv_obj_set_style_bg_color(format_container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(format_container, 0, 0);
    lv_obj_set_style_radius(format_container, 20, 0);
    lv_obj_clear_flag(format_container, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_flex_flow(format_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(format_container,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(format_container, 6, 0);

    ui_Format24Label = lv_label_create(format_container);
    lv_label_set_text(ui_Format24Label, "24H");
    lv_obj_set_style_text_color(ui_Format24Label, lv_color_hex(0x4FC3F7), 0);
    lv_obj_set_style_text_font(ui_Format24Label, &lv_font_montserrat_16, 0);

    ui_FormatSwitch = lv_switch_create(format_container);
    lv_obj_set_size(ui_FormatSwitch, 50, 25);
    lv_obj_add_event_cb(ui_FormatSwitch, ui_event_FormatSwitch, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(ui_FormatSwitch, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_FormatSwitch, lv_color_hex(0x4FC3F7), LV_PART_INDICATOR | LV_STATE_CHECKED);

    ui_Format12Label = lv_label_create(format_container);
    lv_label_set_text(ui_Format12Label, "12H");
    lv_obj_set_style_text_color(ui_Format12Label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_Format12Label, &lv_font_montserrat_16, 0);

    lv_obj_t * buttons_row = lv_obj_create(bottom_area);
    lv_obj_set_size(buttons_row, 450, 60); 
    lv_obj_set_style_bg_opa(buttons_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(buttons_row, 0, 0);
    lv_obj_clear_flag(buttons_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(buttons_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons_row,
        LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); 
    lv_obj_set_style_pad_all(buttons_row, 0, 0);
    lv_obj_align(bottom_area, LV_ALIGN_BOTTOM_MID, 0, -20);

    ui_SpeakButton = lv_btn_create(buttons_row);
    lv_obj_set_size(ui_SpeakButton, 200, 50);
    lv_obj_add_event_cb(ui_SpeakButton, ui_event_SpeakButton, LV_EVENT_ALL, NULL);
    
    lv_obj_set_style_bg_color(ui_SpeakButton, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_SpeakButton, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_SpeakButton, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(ui_SpeakButton, 12, 0);
    lv_obj_clear_flag(ui_SpeakButton, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_state(ui_SpeakButton, LV_STATE_DISABLED);

    ui_SpeakButtonLabel = lv_label_create(ui_SpeakButton);
    lv_label_set_text(ui_SpeakButtonLabel, "SPEAK TIME");
    lv_obj_set_style_text_color(ui_SpeakButtonLabel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_SpeakButtonLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(ui_SpeakButtonLabel, &lv_font_montserrat_18, 0);
    lv_obj_center(ui_SpeakButtonLabel);

    ui_SpeakDateButton = lv_btn_create(buttons_row);
    lv_obj_set_size(ui_SpeakDateButton, 200, 50);
    lv_obj_add_event_cb(ui_SpeakDateButton, ui_event_SpeakDateButton, LV_EVENT_ALL, NULL);
    
    lv_obj_set_style_bg_color(ui_SpeakDateButton, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT); // Более фиолетовый синий
    lv_obj_set_style_bg_color(ui_SpeakDateButton, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_SpeakDateButton, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(ui_SpeakDateButton, 12, 0);
    lv_obj_clear_flag(ui_SpeakDateButton, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_state(ui_SpeakDateButton, LV_STATE_DISABLED);

    ui_SpeakDateButtonLabel = lv_label_create(ui_SpeakDateButton);
    lv_label_set_text(ui_SpeakDateButtonLabel, "SPEAK DATE");
    lv_obj_set_style_text_color(ui_SpeakDateButtonLabel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_SpeakDateButtonLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(ui_SpeakDateButtonLabel, &lv_font_montserrat_18, 0);
    lv_obj_center(ui_SpeakDateButtonLabel);
    if (ui_SpeakButton) {
        lv_obj_set_user_data(ui_SpeakButton, (void*)0);
    }
    if (ui_SpeakDateButton) {
        lv_obj_set_user_data(ui_SpeakDateButton, (void*)0);
    }
    create_navigation_buttons(ui_Screen1);
    create_navigation_dots(ui_Screen1);

    lv_obj_add_event_cb(ui_Screen1, ui_event_Screen1, LV_EVENT_ALL, NULL);
    ESP_LOGI(TAG_CLOCK, "Screen1 clean layout ready");
}

void ui_Screen1_screen_destroy(void)
{
    lv_obj_del(ui_Screen1);
    ui_Screen1 = NULL;
    ui_LeftNavBtn_S1 = NULL; 
    ui_RightNavBtn_S1 = NULL;
    ui_SpeakButton = NULL;
    ui_SpeakButtonLabel = NULL;
    ui_TimeLabel = NULL;
    ui_DateLabel = NULL;
    ui_StatusLabel = NULL;
    ui_FormatSwitch = NULL;
    ui_Format24Label = NULL;
    ui_Format12Label = NULL;
    ui_AnalogClock = NULL;

    ui_hour_hand = NULL;
    ui_minute_hand = NULL;
    ui_second_hand = NULL;
    ui_center_dot = NULL;
    clock_face = NULL;
    ui_SpeakDateButton = NULL;
    ui_SpeakDateButtonLabel = NULL;
    ui_NavDots_S1 = NULL;
    ui_NavDot1_S1 = NULL;
    ui_NavDot2_S1 = NULL;
}
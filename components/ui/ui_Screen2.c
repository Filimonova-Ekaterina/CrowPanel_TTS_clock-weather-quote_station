#include "ui.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include <stdio.h>
#include <math.h>
#include "esp_log.h"

lv_obj_t * ui_Screen2 = NULL;
lv_obj_t * ui_wifiLb = NULL;
lv_obj_t * ui_passLb = NULL;
lv_obj_t * ui_Button1 = NULL;
lv_obj_t * ui_Label3 = NULL;
lv_obj_t * ui_ShowPassBtn = NULL;
lv_obj_t * ui_LeftNavBtn_S2 = NULL;
lv_obj_t * ui_RightNavBtn_S2 = NULL;
static lv_obj_t * ui_NavDots_S2 = NULL;
static lv_obj_t * ui_NavDot1_S2 = NULL;
static lv_obj_t * ui_NavDot2_S2 = NULL;

static const char *TAG_SCREEN2 = "SCREEN2";


void ui_event_LeftNavBtn_S2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN2, "Left navigation button pressed - going to Screen1");
        _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
    }
}
void ui_event_RightNavBtn_S2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN2, "Right navigation button pressed - going to Screen3");
        _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen3_screen_init);
    }
}


static void toggle_password_visibility(lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        extern bool has_wifi_data;
        extern char wifi_password[64];
        extern bool password_visible;
        
        if (!has_wifi_data || wifi_password[0] == '\0') return;
        
        password_visible = !password_visible;
        
        if (ui_passLb) {
            if (password_visible) {
                lv_label_set_text(ui_passLb, wifi_password);
                lv_obj_set_style_text_color(ui_passLb, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
                if (ui_ShowPassBtn) {
                    lv_obj_t * label = lv_obj_get_child(ui_ShowPassBtn, 0);
                    if (label) lv_label_set_text(label, "HIDE");
                }
            } else {
                lv_label_set_text(ui_passLb, "••••••••");
                lv_obj_set_style_text_color(ui_passLb, lv_color_hex(0xE0E0E0), LV_PART_MAIN | LV_STATE_DEFAULT);
                if (ui_ShowPassBtn) {
                    lv_obj_t * label = lv_obj_get_child(ui_ShowPassBtn, 0);
                    if (label) lv_label_set_text(label, "SHOW");
                }
            }
        }
        ESP_LOGI(TAG_SCREEN2, "Password visibility: %s", password_visible ? "visible" : "hidden");
    }
}

void ui_event_Screen2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;
        
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(indev);
        
        if(gesture_dir == LV_DIR_RIGHT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
        }
        else if(gesture_dir == LV_DIR_LEFT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen3_screen_init);
        }
    }
}

static void create_navigation_buttons(lv_obj_t * parent)
{
    ui_LeftNavBtn_S2 = lv_btn_create(parent);
    lv_obj_set_size(ui_LeftNavBtn_S2, 50, 50); 
    lv_obj_align(ui_LeftNavBtn_S2, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S2, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S2, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_LeftNavBtn_S2, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_LeftNavBtn_S2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_LeftNavBtn_S2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_LeftNavBtn_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t * left_arrow = lv_label_create(ui_LeftNavBtn_S2);
    lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(left_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(left_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(left_arrow);
    lv_obj_set_style_pad_all(left_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(left_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_LeftNavBtn_S2, ui_event_LeftNavBtn_S2, LV_EVENT_CLICKED, NULL);
    
    ui_RightNavBtn_S2 = lv_btn_create(parent);
    lv_obj_set_size(ui_RightNavBtn_S2, 50, 50);
    lv_obj_align(ui_RightNavBtn_S2, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S2, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S2, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_RightNavBtn_S2, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_RightNavBtn_S2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_RightNavBtn_S2, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_RightNavBtn_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT); 
    
    lv_obj_t * right_arrow = lv_label_create(ui_RightNavBtn_S2);
    lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(right_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(right_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(right_arrow);
    lv_obj_set_style_pad_all(right_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(right_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_RightNavBtn_S2, ui_event_RightNavBtn_S2, LV_EVENT_CLICKED, NULL);
    
    
}

static void create_navigation_dots(lv_obj_t * parent)
{
    ui_NavDots_S2 = lv_obj_create(parent);
    lv_obj_set_size(ui_NavDots_S2, 80, 20);
    lv_obj_align(ui_NavDots_S2, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(ui_NavDots_S2, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDots_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_NavDots_S2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ui_NavDots_S2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_NavDots_S2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(ui_NavDots_S2, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot1_S2 = lv_obj_create(ui_NavDots_S2);
    lv_obj_set_size(ui_NavDot1_S2, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot1_S2, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot1_S2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot1_S2, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot1_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot2_S2 = lv_obj_create(ui_NavDots_S2);
    lv_obj_set_size(ui_NavDot2_S2, 10, 10);
    lv_obj_set_style_bg_color(ui_NavDot2_S2, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot2_S2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot2_S2, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot2_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    static lv_obj_t * ui_NavDot3_S2 = NULL;
    ui_NavDot3_S2 = lv_obj_create(ui_NavDots_S2);
    lv_obj_set_size(ui_NavDot3_S2, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot3_S2, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot3_S2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot3_S2, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot3_S2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_Screen2_screen_init(void)
{
    ESP_LOGI(TAG_SCREEN2, "Initializing Screen2 (styled like Screen1)...");

    ui_Screen2 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen2, lv_color_hex(0x0D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * top_panel = lv_obj_create(ui_Screen2);
    lv_obj_set_size(top_panel, lv_pct(100), 50);
    lv_obj_align(top_panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_panel, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(top_panel, 0, 0);
    lv_obj_clear_flag(top_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(top_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_panel,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(top_panel, 30, 0);
    lv_obj_set_style_pad_ver(top_panel, 8, 0);

    lv_obj_t * title_label = lv_label_create(top_panel);
    lv_label_set_text(title_label, "Wi-Fi Settings");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);

    lv_obj_t * center_area = lv_obj_create(ui_Screen2);
    lv_obj_set_size(center_area, lv_pct(85), 320);
    lv_obj_align(center_area, LV_ALIGN_CENTER, 0, -10); 
    lv_obj_set_style_bg_opa(center_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center_area, 0, 0);
    lv_obj_clear_flag(center_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(center_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(center_area,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(center_area, 0, 0);
    lv_obj_set_style_pad_row(center_area, 20, 0); 

    lv_obj_t * wifi_container = lv_obj_create(center_area);
    lv_obj_set_size(wifi_container, 400, 70);
    lv_obj_set_style_bg_color(wifi_container, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(wifi_container, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_container, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(wifi_container, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(wifi_container, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(wifi_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(wifi_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wifi_container,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(wifi_container, 15, 0);
    lv_obj_set_style_pad_gap(wifi_container, 15, 0);

    lv_obj_t * wifi_label = lv_label_create(wifi_container);
    lv_label_set_text(wifi_label, "Wi-Fi Network:");
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_16, 0);
    lv_obj_set_width(wifi_label, 130);

    ui_wifiLb = lv_label_create(wifi_container);
    lv_label_set_text(ui_wifiLb, "Enter network name");
    lv_obj_set_style_text_color(ui_wifiLb, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_wifiLb, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(ui_wifiLb, 1);

    lv_obj_t * password_container = lv_obj_create(center_area);
    lv_obj_set_size(password_container, 400, 70); 
    lv_obj_set_style_bg_color(password_container, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(password_container, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(password_container, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(password_container, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(password_container, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(password_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(password_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(password_container,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(password_container, 15, 0);
    lv_obj_set_style_pad_gap(password_container, 10, 0);

    lv_obj_t * pass_label = lv_label_create(password_container);
    lv_label_set_text(pass_label, "Password:");
    lv_obj_set_style_text_color(pass_label, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(pass_label, &lv_font_montserrat_16, 0);
    lv_obj_set_width(pass_label, 130);

    ui_passLb = lv_label_create(password_container);
    lv_label_set_text(ui_passLb, "Enter password");
    lv_obj_set_style_text_color(ui_passLb, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_passLb, &lv_font_montserrat_14, 0);
    lv_obj_set_flex_grow(ui_passLb, 1);

    ui_ShowPassBtn = lv_btn_create(password_container);
    lv_obj_set_size(ui_ShowPassBtn, 80, 35);
    lv_obj_set_style_bg_color(ui_ShowPassBtn, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_ShowPassBtn, lv_color_hex(0x777777), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_ShowPassBtn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(ui_ShowPassBtn, 6, 0);
    
    lv_obj_t * show_pass_label = lv_label_create(ui_ShowPassBtn);
    lv_label_set_text(show_pass_label, "SHOW");
    lv_obj_set_style_text_color(show_pass_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(show_pass_label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(show_pass_label, &lv_font_montserrat_12, 0);
    lv_obj_center(show_pass_label);
    lv_obj_add_event_cb(ui_ShowPassBtn, toggle_password_visibility, LV_EVENT_CLICKED, NULL);

    ui_Button1 = lv_btn_create(center_area);
    lv_obj_set_size(ui_Button1, 180, 45); 
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(ui_Button1, 10, 0);
    lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_SCROLLABLE);
    
    ui_Label3 = lv_label_create(ui_Button1);
    lv_label_set_text(ui_Label3, "CONNECT");
    lv_obj_set_style_text_color(ui_Label3, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Label3, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(ui_Label3, &lv_font_montserrat_16, 0);
    lv_obj_center(ui_Label3);
    lv_obj_add_event_cb(ui_Button1, ui_event_Connect, LV_EVENT_CLICKED, NULL);

    create_navigation_buttons(ui_Screen2);
    create_navigation_dots(ui_Screen2);

    update_buttons_state();

    lv_obj_add_event_cb(ui_Screen2, ui_event_Screen2, LV_EVENT_ALL, NULL);
    ESP_LOGI(TAG_SCREEN2, "Screen2 styled layout ready");
}

void ui_Screen2_screen_destroy(void)
{
    if(ui_Screen2) lv_obj_del(ui_Screen2);

    ui_Screen2 = NULL;
    ui_LeftNavBtn_S2 = NULL; 
    ui_RightNavBtn_S2 = NULL;
    ui_wifiLb = NULL;
    ui_passLb = NULL;
    ui_Button1 = NULL;
    ui_Label3 = NULL;
    ui_ShowPassBtn = NULL;
}
#include "ui.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include <stdio.h>
#include <math.h>
#include "esp_log.h"

lv_obj_t * ui_Screen3 = NULL;
lv_obj_t * ui_CurrentTempLabel = NULL;
lv_obj_t * ui_CurrentWeatherDesc = NULL;
lv_obj_t * ui_FeelsLikeLabel = NULL;
lv_obj_t * ui_HumidityLabel = NULL;
lv_obj_t * ui_PressureLabel = NULL;
lv_obj_t * ui_CityLabel = NULL;
lv_obj_t * ui_WeatherStatusLabel = NULL;
lv_obj_t * ui_SpeakWeatherBtn = NULL;
lv_obj_t * ui_SpeakTodayBtn = NULL;
lv_obj_t * ui_SpeakForecastBtn = NULL;
lv_obj_t * ui_NavDots_S3 = NULL;
lv_obj_t * ui_NavDot1_S3 = NULL;
lv_obj_t * ui_NavDot2_S3 = NULL;
lv_obj_t * ui_NavDot3_S3 = NULL;
lv_obj_t * ui_TimeSlot12 = NULL;
lv_obj_t * ui_TimeSlot17 = NULL;
lv_obj_t * ui_TimeSlot22 = NULL;
lv_obj_t * ui_Day1Forecast = NULL;
lv_obj_t * ui_Day2Forecast = NULL;
lv_obj_t * ui_Day3Forecast = NULL;
lv_obj_t * ui_Time12Temp = NULL;
lv_obj_t * ui_Time12Desc = NULL;
lv_obj_t * ui_Time17Temp = NULL;
lv_obj_t * ui_Time17Desc = NULL;
lv_obj_t * ui_Time22Temp = NULL;
lv_obj_t * ui_Time22Desc = NULL;
lv_obj_t * ui_Day1Temp = NULL;
lv_obj_t * ui_Day1Desc = NULL;
lv_obj_t * ui_Day2Temp = NULL;
lv_obj_t * ui_Day2Desc = NULL;
lv_obj_t * ui_Day3Temp = NULL;
lv_obj_t * ui_Day3Desc = NULL;
lv_obj_t * ui_LeftNavBtn = NULL;
lv_obj_t * ui_RightNavBtn = NULL;
lv_obj_t * ui_UpdateWeatherBtn = NULL;


static const char *TAG_SCREEN3 = "SCREEN3";


void ui_event_Screen3(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;
        
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(indev);
        
        if(gesture_dir == LV_DIR_RIGHT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
        }
        else if(gesture_dir == LV_DIR_LEFT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
        }
    }
}

static void create_navigation_dots(lv_obj_t * parent)
{
    ui_NavDots_S3 = lv_obj_create(parent);
    lv_obj_set_size(ui_NavDots_S3, 80, 20);
    lv_obj_align(ui_NavDots_S3, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(ui_NavDots_S3, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDots_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_NavDots_S3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ui_NavDots_S3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_NavDots_S3, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(ui_NavDots_S3, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot1_S3 = lv_obj_create(ui_NavDots_S3);
    lv_obj_set_size(ui_NavDot1_S3, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot1_S3, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot1_S3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot1_S3, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot1_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot2_S3 = lv_obj_create(ui_NavDots_S3);
    lv_obj_set_size(ui_NavDot2_S3, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot2_S3, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot2_S3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot2_S3, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot2_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot3_S3 = lv_obj_create(ui_NavDots_S3);
    lv_obj_set_size(ui_NavDot3_S3, 10, 10);
    lv_obj_set_style_bg_color(ui_NavDot3_S3, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot3_S3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot3_S3, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot3_S3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_obj_t* create_weather_button(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 130, 46); // Увеличена ширина кнопок
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    
    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    }
    
    return btn;
}

void ui_event_LeftNavBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN3, "Left navigation button pressed - going to Screen2");
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
    }
}

void ui_event_RightNavBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN3, "Right navigation button pressed - going to Screen1");
        _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
    }
}

static void create_navigation_buttons(lv_obj_t * parent)
{
    ui_LeftNavBtn = lv_btn_create(parent);
    lv_obj_set_size(ui_LeftNavBtn, 50, 50); 
    lv_obj_align(ui_LeftNavBtn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(ui_LeftNavBtn, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LeftNavBtn, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_LeftNavBtn, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_LeftNavBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_LeftNavBtn, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_LeftNavBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t * left_arrow = lv_label_create(ui_LeftNavBtn);
    lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(left_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(left_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(left_arrow);
    lv_obj_set_style_pad_all(left_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(left_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_LeftNavBtn, ui_event_LeftNavBtn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_state(ui_UpdateWeatherBtn, LV_STATE_DISABLED);
    
    ui_RightNavBtn = lv_btn_create(parent);
    lv_obj_set_size(ui_RightNavBtn, 50, 50);
    lv_obj_align(ui_RightNavBtn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(ui_RightNavBtn, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_RightNavBtn, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_RightNavBtn, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_RightNavBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_RightNavBtn, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_RightNavBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT); 
    
    lv_obj_t * right_arrow = lv_label_create(ui_RightNavBtn);
    lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(right_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(right_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(right_arrow);
    lv_obj_set_style_pad_all(right_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(right_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_RightNavBtn, ui_event_RightNavBtn, LV_EVENT_CLICKED, NULL);
    
    
}

void ui_Screen3_screen_init(void)
{
    ESP_LOGI(TAG_SCREEN3, "Initializing Screen3 (Weather Station)...");

    ui_Screen3 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen3, lv_color_hex(0x0D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // === Верхняя панель ===
    lv_obj_t * top_panel = lv_obj_create(ui_Screen3);
    lv_obj_set_size(top_panel, lv_pct(100), 50);
    lv_obj_align(top_panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_panel, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(top_panel, 0, 0);
    lv_obj_clear_flag(top_panel, LV_OBJ_FLAG_SCROLLABLE);
       
    ui_UpdateWeatherBtn = lv_btn_create(top_panel);
    lv_obj_set_size(ui_UpdateWeatherBtn, 40, 40);
    lv_obj_align(ui_UpdateWeatherBtn, LV_ALIGN_LEFT_MID, 15, 0);
    lv_obj_set_style_bg_color(ui_UpdateWeatherBtn, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_UpdateWeatherBtn, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_UpdateWeatherBtn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(ui_UpdateWeatherBtn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_UpdateWeatherBtn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_UpdateWeatherBtn, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui_UpdateWeatherBtn, ui_event_UpdateWeatherBtn, LV_EVENT_CLICKED, NULL);

    lv_obj_t* refresh_icon = lv_label_create(ui_UpdateWeatherBtn);
    lv_label_set_text(refresh_icon, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(refresh_icon, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(refresh_icon, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(refresh_icon, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(refresh_icon);
    

    lv_obj_t * title_label = lv_label_create(top_panel);
    lv_label_set_text(title_label, "Weather station");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_center(title_label);

    ui_WeatherStatusLabel = lv_label_create(top_panel);
    lv_label_set_text(ui_WeatherStatusLabel, "Loading...");
    lv_obj_set_style_text_color(ui_WeatherStatusLabel, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_WeatherStatusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(ui_WeatherStatusLabel, LV_ALIGN_RIGHT_MID, -15, 0);

    lv_obj_t * main_area = lv_obj_create(ui_Screen3);
    lv_obj_set_size(main_area, lv_pct(100), 400);
    lv_obj_align(main_area, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_style_bg_opa(main_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_area, 0, 0);
    lv_obj_clear_flag(main_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_area, 0, 0);
    lv_obj_set_style_pad_row(main_area, 12, 0);

    lv_obj_t * current_weather_block = lv_obj_create(main_area);
    lv_obj_set_size(current_weather_block, lv_pct(95), 110); 
    lv_obj_set_style_bg_color(current_weather_block, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(current_weather_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(current_weather_block, 1, 0);
    lv_obj_set_style_border_color(current_weather_block, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(current_weather_block, 12, 0);
    lv_obj_clear_flag(current_weather_block, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(current_weather_block, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(current_weather_block, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * left_info = lv_obj_create(current_weather_block);
    lv_obj_set_size(left_info, lv_pct(35), lv_pct(90)); 
    lv_obj_set_style_bg_opa(left_info, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_info, 0, 0);
    lv_obj_clear_flag(left_info, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(left_info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_info, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(left_info, 15, 0);
    lv_obj_set_style_pad_top(left_info, 12, 0); 
    lv_obj_set_style_pad_bottom(left_info, 12, 0); 

    ui_CityLabel = lv_label_create(left_info);
    lv_label_set_text(ui_CityLabel, "Detecting...");
    lv_obj_set_style_text_color(ui_CityLabel, lv_color_hex(0x4FC3F7), 0);
    lv_obj_set_style_text_font(ui_CityLabel, &lv_font_montserrat_18, 0);

    ui_CurrentWeatherDesc = lv_label_create(left_info);
    lv_label_set_text(ui_CurrentWeatherDesc, "Loading weather...");
    lv_obj_set_style_text_color(ui_CurrentWeatherDesc, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(ui_CurrentWeatherDesc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_top(ui_CurrentWeatherDesc, 0, 0); 

    lv_obj_t * center_temp = lv_obj_create(current_weather_block);
    lv_obj_set_size(center_temp, lv_pct(30), lv_pct(90)); 
    lv_obj_set_style_bg_opa(center_temp, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center_temp, 0, 0);
    lv_obj_clear_flag(center_temp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(center_temp, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(center_temp, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ui_CurrentTempLabel = lv_label_create(center_temp);
    lv_label_set_text(ui_CurrentTempLabel, "--°C");
    lv_obj_set_style_text_color(ui_CurrentTempLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_CurrentTempLabel, &lv_font_montserrat_28, 0);

    lv_obj_t * right_info = lv_obj_create(current_weather_block);
    lv_obj_set_size(right_info, lv_pct(30), lv_pct(90)); 
    lv_obj_set_style_bg_opa(right_info, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_info, 0, 0);
    lv_obj_clear_flag(right_info, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(right_info, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_info, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * details_container = lv_obj_create(right_info);
    lv_obj_set_size(details_container, 150, 85); 
    lv_obj_set_style_bg_opa(details_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(details_container, 0, 0);
    lv_obj_clear_flag(details_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(details_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(details_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * feels_row = lv_obj_create(details_container);
    lv_obj_set_size(feels_row, 160, 28);
    lv_obj_set_style_bg_opa(feels_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(feels_row, 0, 0);
    lv_obj_clear_flag(feels_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(feels_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(feels_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * feels_text = lv_label_create(feels_row);
    lv_label_set_text(feels_text, "Feels like:");
    lv_obj_set_style_text_color(feels_text, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(feels_text, &lv_font_montserrat_12, 0);

    ui_FeelsLikeLabel = lv_label_create(feels_row);
    lv_label_set_text(ui_FeelsLikeLabel, "--°C");
    lv_obj_set_style_text_color(ui_FeelsLikeLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_FeelsLikeLabel, &lv_font_montserrat_12, 0);

    lv_obj_t * humidity_row = lv_obj_create(details_container);
    lv_obj_set_size(humidity_row, 160, 28);
    lv_obj_set_style_bg_opa(humidity_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(humidity_row, 0, 0);
    lv_obj_clear_flag(humidity_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(humidity_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(humidity_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * humidity_text = lv_label_create(humidity_row);
    lv_label_set_text(humidity_text, "Humidity:");
    lv_obj_set_style_text_color(humidity_text, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(humidity_text, &lv_font_montserrat_12, 0);

    ui_HumidityLabel = lv_label_create(humidity_row);
    lv_label_set_text(ui_HumidityLabel, "--%");
    lv_obj_set_style_text_color(ui_HumidityLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_HumidityLabel, &lv_font_montserrat_12, 0);

    lv_obj_t * pressure_row = lv_obj_create(details_container);
    lv_obj_set_size(pressure_row, 160, 28);
    lv_obj_set_style_bg_opa(pressure_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pressure_row, 0, 0);
    lv_obj_clear_flag(pressure_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(pressure_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(pressure_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * pressure_text = lv_label_create(pressure_row);
    lv_label_set_text(pressure_text, "Pressure:");
    lv_obj_set_style_text_color(pressure_text, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(pressure_text, &lv_font_montserrat_12, 0);

    ui_PressureLabel = lv_label_create(pressure_row);
    lv_label_set_text(ui_PressureLabel, "-- hPa");
    lv_obj_set_style_text_color(ui_PressureLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_PressureLabel, &lv_font_montserrat_12, 0);

    lv_obj_t * today_forecast_block = lv_obj_create(main_area);
    lv_obj_set_size(today_forecast_block, lv_pct(95), 100);
    lv_obj_set_style_bg_opa(today_forecast_block, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(today_forecast_block, 0, 0);
    lv_obj_clear_flag(today_forecast_block, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(today_forecast_block, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(today_forecast_block, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * today_title_container = lv_obj_create(today_forecast_block);
    lv_obj_set_size(today_title_container, 100, 90); 
    lv_obj_set_style_bg_opa(today_title_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(today_title_container, 0, 0);
    lv_obj_clear_flag(today_title_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(today_title_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(today_title_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(today_title_container, 30, 0); 

    lv_obj_t * today_title_line1 = lv_label_create(today_title_container);
    lv_label_set_text(today_title_line1, "TODAY'S");
    lv_obj_set_style_text_color(today_title_line1, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(today_title_line1, &lv_font_montserrat_14, 0);

    lv_obj_t * today_title_line2 = lv_label_create(today_title_container);
    lv_label_set_text(today_title_line2, "FORECAST");
    lv_obj_set_style_text_color(today_title_line2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(today_title_line2, &lv_font_montserrat_14, 0);

    lv_obj_t * time_slots = lv_obj_create(today_forecast_block);
    lv_obj_set_size(time_slots, lv_pct(80), 100); 
    lv_obj_set_style_bg_opa(time_slots, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_slots, 0, 0);
    lv_obj_clear_flag(time_slots, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(time_slots, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(time_slots, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(time_slots, 20, 0);

    for (int i = 0; i < 3; i++) {
        lv_obj_t* slot = lv_obj_create(time_slots);
        lv_obj_set_size(slot, 165, 85); 
        lv_obj_set_style_bg_color(slot, lv_color_hex(0x1A1A1A), 0);
        lv_obj_set_style_bg_opa(slot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(slot, 1, 0);
        lv_obj_set_style_border_color(slot, lv_color_hex(0x333333), 0);
        lv_obj_set_style_radius(slot, 10, 0);
        lv_obj_clear_flag(slot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(slot, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(slot, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(slot, 6, 0);

        lv_obj_t* time_label = lv_label_create(slot);
        const char* times[] = {"9:00", "15:00", "21:00"};
        lv_label_set_text(time_label, times[i]);
        lv_obj_set_style_text_color(time_label, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);

        lv_obj_t* info_row = lv_obj_create(slot);
        lv_obj_set_size(info_row, 165, 25);
        lv_obj_set_style_bg_opa(info_row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info_row, 0, 0);
        lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(info_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(info_row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* temp_label = lv_label_create(info_row);
        lv_label_set_text(temp_label, "--°");
        lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_16, 0);

        lv_obj_t* desc_label = lv_label_create(info_row);
        lv_label_set_text(desc_label, "--");
        lv_obj_set_style_text_color(desc_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(desc_label, &lv_font_montserrat_12, 0);

        if (i == 0) {
            ui_TimeSlot12 = slot;
            ui_Time12Temp = temp_label;
            ui_Time12Desc = desc_label;
        } else if (i == 1) {
            ui_TimeSlot17 = slot;
            ui_Time17Temp = temp_label;
            ui_Time17Desc = desc_label;
        } else if (i == 2) {
            ui_TimeSlot22 = slot;
            ui_Time22Temp = temp_label;
            ui_Time22Desc = desc_label;
        }
    }

    lv_obj_t * three_day_block = lv_obj_create(main_area);
    lv_obj_set_size(three_day_block, lv_pct(95), 100);
    lv_obj_set_style_bg_color(three_day_block, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(three_day_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(three_day_block, 1, 0);
    lv_obj_set_style_border_color(three_day_block, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(three_day_block, 12, 0);
    lv_obj_clear_flag(three_day_block, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(three_day_block, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(three_day_block, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * forecast_title_container = lv_obj_create(three_day_block);
    lv_obj_set_size(forecast_title_container, 100, 90); 
    lv_obj_set_style_bg_opa(forecast_title_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(forecast_title_container, 0, 0);
    lv_obj_clear_flag(forecast_title_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(forecast_title_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(forecast_title_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(forecast_title_container, 30, 0); 

    lv_obj_t * forecast_title_line1 = lv_label_create(forecast_title_container);
    lv_label_set_text(forecast_title_line1, "3-DAY");
    lv_obj_set_style_text_color(forecast_title_line1, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(forecast_title_line1, &lv_font_montserrat_14, 0);

    lv_obj_t * forecast_title_line2 = lv_label_create(forecast_title_container);
    lv_label_set_text(forecast_title_line2, "FORECAST");
    lv_obj_set_style_text_color(forecast_title_line2, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(forecast_title_line2, &lv_font_montserrat_14, 0);

    lv_obj_t * days_container = lv_obj_create(three_day_block);
    lv_obj_set_size(days_container, lv_pct(80), 100); 
    lv_obj_set_style_bg_opa(days_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(days_container, 0, 0);
    lv_obj_clear_flag(days_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(days_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(days_container, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(days_container, 20, 0); 

    for (int i = 0; i < 3; i++) {
        lv_obj_t* day_slot = lv_obj_create(days_container);
        lv_obj_set_size(day_slot, 165, 75);
        lv_obj_set_style_bg_opa(day_slot, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(day_slot, 0, 0);
        lv_obj_clear_flag(day_slot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(day_slot, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(day_slot, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(day_slot, 5, 0);

        lv_obj_t* day_label = lv_label_create(day_slot);
        const char* days[] = {"TOMORROW", "DAY 2", "DAY 3"};
        lv_label_set_text(day_label, days[i]);
        lv_obj_set_style_text_color(day_label, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(day_label, &lv_font_montserrat_14, 0);

        lv_obj_t* info_row = lv_obj_create(day_slot);
        lv_obj_set_size(info_row, 165, 25);
        lv_obj_set_style_bg_opa(info_row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(info_row, 0, 0);
        lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_flex_flow(info_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(info_row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t* temp_label = lv_label_create(info_row);
        lv_label_set_text(temp_label, "--°");
        lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_16, 0);

        lv_obj_t* desc_label = lv_label_create(info_row);
        lv_label_set_text(desc_label, "---");
        lv_obj_set_style_text_color(desc_label, lv_color_hex(0xE0E0E0), 0);
        lv_obj_set_style_text_font(desc_label, &lv_font_montserrat_12, 0);

        if (i == 0) {
            ui_Day1Forecast = day_slot;
            ui_Day1Temp = temp_label;
            ui_Day1Desc = desc_label;
        } else if (i == 1) {
            ui_Day2Forecast = day_slot;
            ui_Day2Temp = temp_label;
            ui_Day2Desc = desc_label;
        } else if (i == 2) {
            ui_Day3Forecast = day_slot;
            ui_Day3Temp = temp_label;
            ui_Day3Desc = desc_label;
        }
    }

    lv_obj_t * buttons_panel = lv_obj_create(ui_Screen3);
    lv_obj_set_size(buttons_panel, lv_pct(95), 60);
    lv_obj_align(buttons_panel, LV_ALIGN_BOTTOM_MID, 0, -25);
    lv_obj_set_style_bg_opa(buttons_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(buttons_panel, 0, 0);
    lv_obj_clear_flag(buttons_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(buttons_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons_panel, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ui_SpeakWeatherBtn = create_weather_button(buttons_panel, "CURRENT", ui_event_SpeakWeatherBtn);
    lv_obj_set_width(ui_SpeakWeatherBtn, 125);

    ui_SpeakTodayBtn = create_weather_button(buttons_panel, "TODAY", ui_event_SpeakTodayBtn);
    lv_obj_set_width(ui_SpeakTodayBtn, 125);

    ui_SpeakForecastBtn = create_weather_button(buttons_panel, "3-DAY", ui_event_SpeakForecastBtn);
    lv_obj_set_width(ui_SpeakForecastBtn, 125);
    
    create_navigation_buttons(ui_Screen3);
    create_navigation_dots(ui_Screen3);

    lock_weather_buttons();

    lv_obj_add_event_cb(ui_Screen3, ui_event_Screen3, LV_EVENT_ALL, NULL);
    ESP_LOGI(TAG_SCREEN3, "Screen3 Weather Station ready");
}

void ui_Screen3_screen_destroy(void)
{
    if(ui_Screen3) lv_obj_del(ui_Screen3);

    ui_Screen3 = NULL;
    ui_LeftNavBtn = NULL; 
    ui_RightNavBtn = NULL; 
    ui_CurrentTempLabel = NULL;
    ui_CurrentWeatherDesc = NULL;
    ui_FeelsLikeLabel = NULL;
    ui_HumidityLabel = NULL;
    ui_PressureLabel = NULL;
    ui_CityLabel = NULL;
    ui_WeatherStatusLabel = NULL;
    ui_SpeakWeatherBtn = NULL;
    ui_SpeakTodayBtn = NULL;
    ui_SpeakForecastBtn = NULL;
    ui_NavDots_S3 = NULL;
    ui_NavDot1_S3 = NULL;
    ui_NavDot2_S3 = NULL;
    ui_NavDot3_S3 = NULL;
    ui_TimeSlot12 = NULL;
    ui_TimeSlot17 = NULL;
    ui_TimeSlot22 = NULL;
    ui_Day1Forecast = NULL;
    ui_Day2Forecast = NULL;
    ui_Day3Forecast = NULL;
    ui_Time12Temp = NULL;
    ui_Time12Desc = NULL;
    ui_Time17Temp = NULL;
    ui_Time17Desc = NULL;
    ui_Time22Temp = NULL;
    ui_Time22Desc = NULL;
    ui_Day1Temp = NULL;
    ui_Day1Desc = NULL;
    ui_Day2Temp = NULL;
    ui_Day2Desc = NULL;
    ui_Day3Temp = NULL;
    ui_Day3Desc = NULL;
}
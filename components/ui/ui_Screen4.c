#include "ui.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include <stdio.h>
#include "esp_log.h"

lv_obj_t * ui_Screen4 = NULL;
lv_obj_t * ui_LeftNavBtn_S4 = NULL;
lv_obj_t * ui_RightNavBtn_S4 = NULL;
lv_obj_t * ui_QuoteStatusLabel = NULL;
lv_obj_t * ui_QuoteTextArea = NULL;
lv_obj_t * ui_UpdateQuoteBtn = NULL;
lv_obj_t * ui_SpeakQuoteBtn = NULL;
lv_obj_t * ui_NavDots_S4 = NULL;
lv_obj_t * ui_NavDot1_S4 = NULL;
lv_obj_t * ui_NavDot2_S4 = NULL;
lv_obj_t * ui_NavDot3_S4 = NULL;
lv_obj_t * ui_NavDot4_S4 = NULL;

static const char *TAG_SCREEN4 = "SCREEN4";

void ui_event_Screen4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE) {
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;
        
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(indev);
        
        if(gesture_dir == LV_DIR_RIGHT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen3_screen_init);
        }
        else if(gesture_dir == LV_DIR_LEFT) {
            lv_indev_wait_release(indev);
            _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
        }
    }
}

void ui_event_LeftNavBtn_S4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN4, "Left navigation button pressed - going to Screen3");
        _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen3_screen_init);
    }
}

void ui_event_RightNavBtn_S4(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG_SCREEN4, "Right navigation button pressed - going to Screen1");
        _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
    }
}

static void create_navigation_dots(lv_obj_t * parent)
{
    ui_NavDots_S4 = lv_obj_create(parent);
    lv_obj_set_size(ui_NavDots_S4, 100, 20);
    lv_obj_align(ui_NavDots_S4, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(ui_NavDots_S4, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDots_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_NavDots_S4, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(ui_NavDots_S4, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_NavDots_S4, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(ui_NavDots_S4, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot1_S4 = lv_obj_create(ui_NavDots_S4);
    lv_obj_set_size(ui_NavDot1_S4, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot1_S4, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot1_S4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot1_S4, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot1_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot2_S4 = lv_obj_create(ui_NavDots_S4);
    lv_obj_set_size(ui_NavDot2_S4, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot2_S4, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot2_S4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot2_S4, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot2_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot3_S4 = lv_obj_create(ui_NavDots_S4);
    lv_obj_set_size(ui_NavDot3_S4, 8, 8);
    lv_obj_set_style_bg_color(ui_NavDot3_S4, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot3_S4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot3_S4, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot3_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_NavDot4_S4 = lv_obj_create(ui_NavDots_S4);
    lv_obj_set_size(ui_NavDot4_S4, 10, 10);
    lv_obj_set_style_bg_color(ui_NavDot4_S4, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_NavDot4_S4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_NavDot4_S4, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_NavDot4_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void create_navigation_buttons(lv_obj_t * parent)
{
    ui_LeftNavBtn_S4 = lv_btn_create(parent);
    lv_obj_set_size(ui_LeftNavBtn_S4, 50, 50); 
    lv_obj_align(ui_LeftNavBtn_S4, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S4, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_LeftNavBtn_S4, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_LeftNavBtn_S4, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_LeftNavBtn_S4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_LeftNavBtn_S4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_LeftNavBtn_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_t * left_arrow = lv_label_create(ui_LeftNavBtn_S4);
    lv_label_set_text(left_arrow, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(left_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(left_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(left_arrow);
    lv_obj_set_style_pad_all(left_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(left_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_LeftNavBtn_S4, ui_event_LeftNavBtn_S4, LV_EVENT_CLICKED, NULL);
    
    ui_RightNavBtn_S4 = lv_btn_create(parent);
    lv_obj_set_size(ui_RightNavBtn_S4, 50, 50);
    lv_obj_align(ui_RightNavBtn_S4, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S4, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_RightNavBtn_S4, lv_color_hex(0x3A3A3A), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(ui_RightNavBtn_S4, 25, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_style_border_width(ui_RightNavBtn_S4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_RightNavBtn_S4, lv_color_hex(0x444444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui_RightNavBtn_S4, 0, LV_PART_MAIN | LV_STATE_DEFAULT); 
    
    lv_obj_t * right_arrow = lv_label_create(ui_RightNavBtn_S4);
    lv_label_set_text(right_arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(right_arrow, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(right_arrow, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_center(right_arrow);
    lv_obj_set_style_pad_all(right_arrow, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(right_arrow, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(ui_RightNavBtn_S4, ui_event_RightNavBtn_S4, LV_EVENT_CLICKED, NULL);
}

static lv_obj_t* create_quote_button(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 200, 50); // Увеличен размер как на первом экране
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3F9CC6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1A759E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_radius(btn, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_state(btn, LV_STATE_DISABLED);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT); // Увеличен шрифт
    lv_obj_center(label);
    
    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    }
    
    return btn;
}

void ui_Screen4_screen_init(void)
{
    ESP_LOGI(TAG_SCREEN4, "Initializing Screen4 (Quote station)...");

    ui_Screen4 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen4, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Screen4, lv_color_hex(0x0D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen4, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * top_panel = lv_obj_create(ui_Screen4);
    lv_obj_set_size(top_panel, lv_pct(100), 50);
    lv_obj_align(top_panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_panel, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_width(top_panel, 0, 0);
    lv_obj_clear_flag(top_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title_label = lv_label_create(top_panel);
    lv_label_set_text(title_label, "Quote station");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, 0);
    lv_obj_center(title_label);

    ui_QuoteStatusLabel = lv_label_create(top_panel);
    lv_label_set_text(ui_QuoteStatusLabel, "Ready");
    lv_obj_set_style_text_color(ui_QuoteStatusLabel, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ui_QuoteStatusLabel, &lv_font_montserrat_12, 0);
    lv_obj_align(ui_QuoteStatusLabel, LV_ALIGN_RIGHT_MID, -15, 0);

    lv_obj_t * main_area = lv_obj_create(ui_Screen4);
    lv_obj_set_size(main_area, lv_pct(95), 380);
    lv_obj_align(main_area, LV_ALIGN_CENTER, 0, 70);
    lv_obj_set_style_bg_opa(main_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_area, 0, 0);
    lv_obj_clear_flag(main_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_area, 0, 0);
    lv_obj_set_style_pad_row(main_area, 20, 0);

    lv_obj_t * quote_container = lv_obj_create(main_area);
    lv_obj_set_size(quote_container, lv_pct(100), 200);
    lv_obj_set_style_bg_color(quote_container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(quote_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(quote_container, 1, 0);
    lv_obj_set_style_border_color(quote_container, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(quote_container, 12, 0);
    lv_obj_clear_flag(quote_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(quote_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(quote_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * text_container = lv_obj_create(quote_container);
    lv_obj_set_size(text_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(text_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(text_container, 0, 0);
    lv_obj_clear_flag(text_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(text_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    ui_QuoteTextArea = lv_textarea_create(text_container);
    lv_obj_set_size(ui_QuoteTextArea, lv_pct(95), LV_SIZE_CONTENT); 
    lv_textarea_set_text(ui_QuoteTextArea, "Connect to Wi-Fi to get a quote!");
    lv_textarea_set_placeholder_text(ui_QuoteTextArea, "Waiting for quotes...");
    lv_obj_set_style_text_color(ui_QuoteTextArea, lv_color_white(), 0);
    lv_obj_set_style_text_font(ui_QuoteTextArea, &lv_font_montserrat_24, 0);
    lv_obj_set_style_bg_opa(ui_QuoteTextArea, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ui_QuoteTextArea, 0, 0);
    lv_textarea_set_max_length(ui_QuoteTextArea, 1024);
    lv_textarea_set_one_line(ui_QuoteTextArea, false);
    lv_textarea_set_text_selection(ui_QuoteTextArea, false);
    
    // Делаем текстовое поле неактивным - нельзя тыкать
    lv_obj_add_flag(ui_QuoteTextArea, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_QuoteTextArea, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    
    lv_obj_set_style_pad_all(ui_QuoteTextArea, 10, 0);
    lv_obj_set_style_text_align(ui_QuoteTextArea, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_style_pad_all(text_container, 0, 0);
    lv_obj_set_style_pad_top(ui_QuoteTextArea, 0, 0);
    lv_obj_set_style_pad_bottom(ui_QuoteTextArea, 0, 0); 

    lv_obj_t * buttons_panel = lv_obj_create(main_area);
    lv_obj_set_size(buttons_panel, lv_pct(100), 60);
    lv_obj_set_style_bg_opa(buttons_panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(buttons_panel, 0, 0);
    lv_obj_clear_flag(buttons_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(buttons_panel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons_panel, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Более близкое расположение

    ui_UpdateQuoteBtn = create_quote_button(buttons_panel, "UPDATE QUOTE", ui_event_UpdateQuoteBtn);
    ui_SpeakQuoteBtn = create_quote_button(buttons_panel, "SPEAK QUOTE", ui_event_SpeakQuoteBtn);
    
    create_navigation_buttons(ui_Screen4);
    create_navigation_dots(ui_Screen4);

    lv_obj_add_event_cb(ui_Screen4, ui_event_Screen4, LV_EVENT_ALL, NULL);
    ESP_LOGI(TAG_SCREEN4, "Screen4 Quote Station ready");
}

void ui_Screen4_screen_destroy(void)
{
    if(ui_Screen4) lv_obj_del(ui_Screen4);

    ui_Screen4 = NULL;
    ui_LeftNavBtn_S4 = NULL;
    ui_RightNavBtn_S4 = NULL;
    ui_QuoteStatusLabel = NULL;
    ui_QuoteTextArea = NULL;
    ui_UpdateQuoteBtn = NULL;
    ui_SpeakQuoteBtn = NULL;
    ui_NavDots_S4 = NULL;
    ui_NavDot1_S4 = NULL;
    ui_NavDot2_S4 = NULL;
    ui_NavDot3_S4 = NULL;
    ui_NavDot4_S4 = NULL;
}
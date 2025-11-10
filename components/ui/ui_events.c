#include "ui_events.h"
#include "ui.h"           
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "esp_log.h"
#include "tts_bridge.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ui_events";

extern void speak_current_time(void);
extern void speak_current_date(void);
extern void update_time(void);
extern void time_formatter_set_12h_format(bool enabled);
extern void update_analog_clock(int hours, int minutes, int seconds);
extern void speak_current_weather(void);
extern void speak_today_forecast(void);
extern void speak_days_forecast(int days);
extern void request_weather_update(void);
extern bool time_initialized;
extern lv_obj_t *ui_Time12Temp;
extern lv_obj_t *ui_Time12Desc;
extern lv_obj_t *ui_Time17Temp;
extern lv_obj_t *ui_Time17Desc;
extern lv_obj_t *ui_Time22Temp;
extern lv_obj_t *ui_Time22Desc;
extern lv_obj_t *ui_Day1Temp;
extern lv_obj_t *ui_Day1Desc;
extern lv_obj_t *ui_Day2Temp;
extern lv_obj_t *ui_Day2Desc;
extern lv_obj_t *ui_Day3Temp;
extern lv_obj_t *ui_Day3Desc;
extern lv_obj_t * ui_SpeakWeatherBtn;
extern lv_obj_t * ui_SpeakTodayBtn;
extern lv_obj_t * ui_SpeakForecastBtn;
extern lv_obj_t * ui_UpdateWeatherBtn;
extern lv_obj_t * ui_WeatherStatusLabel;
char wifi_ssid[64] = {0};
char wifi_password[64] = {0};
bool has_wifi_data = false;
bool password_visible = false;
extern bool wifi_connected;
static bool speaking_time_date = false;
static bool speaking_weather = false;

void update_weather_status(const char* status, bool is_error) {
    if (ui_WeatherStatusLabel && status) {
        lv_label_set_text(ui_WeatherStatusLabel, status);
        if (is_error) {
            lv_obj_set_style_text_color(ui_WeatherStatusLabel, lv_color_hex(0x888888), 0);
        } else {
            lv_obj_set_style_text_color(ui_WeatherStatusLabel, lv_color_hex(0x888888), 0);
        }
        lv_task_handler(); 
        ESP_LOGI("UI", "Weather status: %s", status);
    }
}

static void update_format_switch_colors(bool is_12h) {
    if (is_12h) {
        lv_obj_set_style_text_color(ui_Format12Label, lv_color_hex(0x4FC3F7), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_Format24Label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_text_color(ui_Format24Label, lv_color_hex(0x4FC3F7), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(ui_Format12Label, lv_color_hex(0x888888), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void lock_weather_buttons(void) {
    if (ui_SpeakWeatherBtn) {
        lv_obj_add_state(ui_SpeakWeatherBtn, LV_STATE_DISABLED);
    }
    if (ui_SpeakTodayBtn) {
        lv_obj_add_state(ui_SpeakTodayBtn, LV_STATE_DISABLED);
    }
    if (ui_SpeakForecastBtn) {
        lv_obj_add_state(ui_SpeakForecastBtn, LV_STATE_DISABLED);
    }
    ESP_LOGI(TAG, "Weather buttons locked");
}


void ui_event_FormatSwitch(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
    
    if(event_code == LV_EVENT_VALUE_CHANGED) {
        bool enabled = lv_obj_has_state(target, LV_STATE_CHECKED);
        time_formatter_set_12h_format(enabled);
        ESP_LOGI(TAG, "Time format changed to: %s", enabled ? "12H" : "24H");
        update_format_switch_colors(enabled);
        update_time();
    }
}

static void disable_other_button_async(void *arg)
{
    lv_obj_t *clicked_button = (lv_obj_t *)arg;

    if (clicked_button == ui_SpeakButton) {

        if (ui_SpeakButton) {
            lv_obj_add_state(ui_SpeakButton, LV_STATE_DISABLED);
        }
        if (ui_SpeakDateButton) {
            lv_obj_clear_flag(ui_SpeakDateButton, LV_OBJ_FLAG_CLICKABLE);
        }
    } 
    else if (clicked_button == ui_SpeakDateButton) {
        if (ui_SpeakDateButton) {
            lv_obj_add_state(ui_SpeakDateButton, LV_STATE_DISABLED);
        }
        if (ui_SpeakButton) {
            lv_obj_clear_flag(ui_SpeakButton, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    
    ESP_LOGI(TAG, "One button visibly locked, other invisibly locked (non-clickable)");
}

void ui_event_SpeakButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        if (!time_initialized) {
            ESP_LOGW(TAG, "Speak button pressed but time not initialized yet");
            return;
        }
        
        ESP_LOGI(TAG, "Speak Time button pressed - making date button non-clickable");
        speaking_time_date = true;
        speaking_weather = false;
        lv_async_call(disable_other_button_async, ui_SpeakButton);
        speak_current_time();
    }
}

void ui_event_SpeakDateButton(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        if (!time_initialized) {
            ESP_LOGW(TAG, "Speak date button pressed but time not initialized yet");
            return;
        }
        
        ESP_LOGI(TAG, "Speak Date button pressed - making time button non-clickable");
        speaking_time_date = true;
        speaking_weather = false;
        lv_async_call(disable_other_button_async, ui_SpeakDateButton);
        speak_current_date();
    }
}

void lock_speak_buttons(void) {
    if (ui_SpeakButton) {
        lv_obj_add_state(ui_SpeakButton, LV_STATE_DISABLED);
    }
    if (ui_SpeakDateButton) {
        lv_obj_add_state(ui_SpeakDateButton, LV_STATE_DISABLED);
    }
    ESP_LOGI(TAG, "Speak buttons locked during WiFi/NTP processing");
}

void unlock_speak_button(void) {
    if (ui_SpeakButton) {
        lv_obj_clear_state(ui_SpeakButton, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakButton, LV_OBJ_FLAG_CLICKABLE);
        if (ui_SpeakButtonLabel) {
            lv_label_set_text(ui_SpeakButtonLabel, "SPEAK TIME");
        }
    }
    
    if (ui_SpeakDateButton) {
        lv_obj_clear_state(ui_SpeakDateButton, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakDateButton, LV_OBJ_FLAG_CLICKABLE);
        if (ui_SpeakDateButtonLabel) {
            lv_label_set_text(ui_SpeakDateButtonLabel, "SPEAK DATE");
        }
    }
    
    ESP_LOGI(TAG, "Both buttons fully unlocked and clickable");
}

void update_status_display(const char* status) {
    if (ui_StatusLabel && status) {
        lv_label_set_text(ui_StatusLabel, status);
    }
}

static void update_time_display_task(void * arg) {
    char *data = (char*)arg;
    if (!data) return;
    
    char *time_str = data;
    char *date_str = strchr(data, '|');
    if (date_str) {
        *date_str = '\0';
        date_str++;
        
        if (ui_TimeLabel) {
            lv_label_set_text(ui_TimeLabel, time_str);
        }
        if (ui_DateLabel) {
            lv_label_set_text(ui_DateLabel, date_str);
        }
    }
    
    free(data);
}

void update_time_display(const char* time_str, const char* date_str)
{
    if (!time_str || !date_str) return;
    
    if (!ui_TimeLabel || !ui_DateLabel) {
        ESP_LOGW(TAG, "UI objects not initialized yet");
        return;
    }
    
    size_t total_len = strlen(time_str) + strlen(date_str) + 2;
    char *copy = (char*)malloc(total_len);
    if (!copy) {
        ESP_LOGE(TAG, "Failed to allocate memory for time display");
        return;
    }
    
    snprintf(copy, total_len, "%s|%s", time_str, date_str);
    lv_async_call(update_time_display_task, copy);
}

void update_buttons_state(void) {
    if (ui_ShowPassBtn) {
        if (has_wifi_data && wifi_password[0] != '\0') {
            lv_obj_clear_state(ui_ShowPassBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(ui_ShowPassBtn, LV_STATE_DISABLED);
        }
    }
    
    if (ui_Button1) {
        if (has_wifi_data && wifi_ssid[0] != '\0') {
            lv_obj_clear_state(ui_Button1, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(ui_Button1, LV_STATE_DISABLED);
        }
    }
}

void unlock_connect_button(void) {
    if (ui_Button1) {
        lv_obj_clear_state(ui_Button1, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_CLICKABLE);  // Дополнительная защита
    }
    
    if (ui_Label3) {
        lv_label_set_text(ui_Label3, "CONNECT");
    }
    
    if (ui_ShowPassBtn) {
        lv_obj_clear_state(ui_ShowPassBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_ShowPassBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    extern bool wifi_connection_in_progress;
    wifi_connection_in_progress = false;
    
    ESP_LOGI(TAG, "Connect button fully unlocked");
}

static void connect_to_wifi_delayed(void *arg) {
    vTaskDelay(300 / portTICK_PERIOD_MS);
    
    extern void connect_to_wifi_manual(const char* ssid, const char* password);
    connect_to_wifi_manual(wifi_ssid, wifi_password);
}

void ui_connect_to_wifi(void) {
    if (wifi_ssid[0] == '\0') {
        ESP_LOGE(TAG, "No SSID set for WiFi connection");
        return;
    }
    
    extern bool wifi_connection_in_progress;
    if (wifi_connection_in_progress) {
        ESP_LOGW(TAG, "WiFi connection already in progress");
        return;
    }
    
    ESP_LOGI(TAG, "Attempting to connect to WiFi: %s", wifi_ssid);
    
    if (ui_Button1) {
        lv_obj_add_state(ui_Button1, LV_STATE_DISABLED);
    }
    if (ui_ShowPassBtn) {
        lv_obj_add_state(ui_ShowPassBtn, LV_STATE_DISABLED);
    }
    
    if (ui_Label3) {
        lv_label_set_text(ui_Label3, "CONNECTING...");
    }
    if (ui_StatusLabel) {
        lv_label_set_text(ui_StatusLabel, "Connecting to WiFi...");
    }
    for (int i = 0; i < 3; i++) {
        lv_task_handler();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    
    lv_async_call(connect_to_wifi_delayed, NULL);
    
    ESP_LOGI(TAG, "WiFi connection process started (async)");
}

void ui_set_wifi_credentials(const char* ssid, const char* password) {
    if (ssid) {
        strncpy(wifi_ssid, ssid, sizeof(wifi_ssid) - 1);
        wifi_ssid[sizeof(wifi_ssid) - 1] = '\0';
    }
    if (password) {
        strncpy(wifi_password, password, sizeof(wifi_password) - 1);
        wifi_password[sizeof(wifi_password) - 1] = '\0';
    }
    
    ESP_LOGI(TAG, "WiFi credentials updated: SSID=%s", wifi_ssid);
    
    has_wifi_data = (wifi_ssid[0] != '\0' || wifi_password[0] != '\0');
    
    if (ui_wifiLb) {
        lv_label_set_text(ui_wifiLb, wifi_ssid[0] ? wifi_ssid : "Enter network name");
        lv_obj_set_style_text_color(ui_wifiLb, 
            wifi_ssid[0] ? lv_color_white() : lv_color_hex(0x888888), 
            LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (ui_passLb) {
        lv_label_set_text(ui_passLb, wifi_password[0] ? "••••••••" : "Enter password");
        lv_obj_set_style_text_color(ui_passLb, 
            wifi_password[0] ? lv_color_hex(0xE0E0E0) : lv_color_hex(0x888888), 
            LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    update_buttons_state();
    
    password_visible = false;
    if (ui_ShowPassBtn) {
        lv_obj_t * label = lv_obj_get_child(ui_ShowPassBtn, 0);
        if (label) lv_label_set_text(label, "SHOW");
    }
    
    unlock_connect_button();
}

void ui_event_Connect(lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Connect button pressed");
        extern int retry_num;
        retry_num = 0;
        ui_connect_to_wifi();
    }
}


static void disable_other_weather_buttons_async(void *arg)
{
    lv_obj_t *clicked_button = (lv_obj_t *)arg;
    if (ui_UpdateWeatherBtn) {
        lv_obj_add_state(ui_UpdateWeatherBtn, LV_STATE_DISABLED);
    }
    // Блокируем все кнопки произношения
    if (ui_SpeakWeatherBtn) {
        if (clicked_button == ui_SpeakWeatherBtn) {
            lv_obj_add_state(ui_SpeakWeatherBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_flag(ui_SpeakWeatherBtn, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    
    if (ui_SpeakTodayBtn) {
        if (clicked_button == ui_SpeakTodayBtn) {
            lv_obj_add_state(ui_SpeakTodayBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_flag(ui_SpeakTodayBtn, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    
    if (ui_SpeakForecastBtn) {
        if (clicked_button == ui_SpeakForecastBtn) {
            lv_obj_add_state(ui_SpeakForecastBtn, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_flag(ui_SpeakForecastBtn, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    
    ESP_LOGI(TAG, "Weather speak buttons: one visibly locked, others invisibly locked");
}

void ui_event_SpeakWeatherBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Speak Current Weather button pressed");
        speaking_time_date = false;
        speaking_weather = true;
        lv_async_call(disable_other_weather_buttons_async, ui_SpeakWeatherBtn);
        speak_current_weather();
    }
}

void ui_event_SpeakTodayBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Speak Today Forecast button pressed");
        speaking_time_date = false;
        speaking_weather = true;
        lv_async_call(disable_other_weather_buttons_async, ui_SpeakTodayBtn);
        speak_today_forecast();
    }
}

void ui_event_SpeakForecastBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Speak 3-Day Forecast button pressed");
        speaking_time_date = false;
        speaking_weather = true;
        lv_async_call(disable_other_weather_buttons_async, ui_SpeakForecastBtn);
        speak_days_forecast(3);
    }
}

void unlock_weather_speak_buttons(void) {
    if (ui_UpdateWeatherBtn) {
        lv_obj_clear_state(ui_UpdateWeatherBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_UpdateWeatherBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    if (ui_SpeakWeatherBtn) {
        lv_obj_clear_state(ui_SpeakWeatherBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakWeatherBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    
    if (ui_SpeakTodayBtn) {
        lv_obj_clear_state(ui_SpeakTodayBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakTodayBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    
    if (ui_SpeakForecastBtn) {
        lv_obj_clear_state(ui_SpeakForecastBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakForecastBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    
    ESP_LOGI(TAG, "All weather speak buttons fully unlocked");
}

void unlock_current_weather_button(void) {
    if (ui_SpeakWeatherBtn) {
        lv_obj_clear_state(ui_SpeakWeatherBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakWeatherBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    ESP_LOGI(TAG, "Current weather button unlocked");
}

void unlock_forecast_buttons(void) {
    if (ui_SpeakTodayBtn) {
        lv_obj_clear_state(ui_SpeakTodayBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakTodayBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    
    if (ui_SpeakForecastBtn) {
        lv_obj_clear_state(ui_SpeakForecastBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_SpeakForecastBtn, LV_OBJ_FLAG_CLICKABLE);
    }
    ESP_LOGI(TAG, "Forecast buttons unlocked");
}

void ui_event_UpdateWeatherBtn(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    
    if(event_code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Update Weather button pressed - requesting update");
        
        // Меняем статус
        update_weather_status("Updating...", false);
        lock_weather_buttons();
        // Принудительная обработка UI
        for (int i = 0; i < 3; i++) {
            lv_task_handler();
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        
        // ЗАПРАШИВАЕМ обновление (вместо прямого вызова)
        request_weather_update();
    }
}


void unlock_update_button_async(void)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    if (ui_UpdateWeatherBtn) {
        lv_obj_clear_state(ui_UpdateWeatherBtn, LV_STATE_DISABLED);
        lv_obj_add_flag(ui_UpdateWeatherBtn, LV_OBJ_FLAG_CLICKABLE);
    }
}


void update_today_forecast_display(float temp_9, float temp_15, float temp_21, 
                                  const char* desc_9, const char* desc_15, const char* desc_21) {
    ESP_LOGI("UI", "Updating today forecast: 9:00=%.1f, 15:00=%.1f, 21:00=%.1f", temp_9, temp_15, temp_21);
    if (ui_Time12Temp) {
        char temp_str[16];
        if (temp_9 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_9);
        }
        lv_label_set_text(ui_Time12Temp, temp_str);
    }
    if (ui_Time12Desc && desc_9) {
        lv_label_set_text(ui_Time12Desc, desc_9);
    }
    if (ui_Time17Temp) {
        char temp_str[16];
        if (temp_15 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_15);
        }
        lv_label_set_text(ui_Time17Temp, temp_str);
    }
    if (ui_Time17Desc && desc_15) {
        lv_label_set_text(ui_Time17Desc, desc_15);
    }
    
    if (ui_Time22Temp) {
        char temp_str[16];
        if (temp_21 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_21);
        }
        lv_label_set_text(ui_Time22Temp, temp_str);
    }
    if (ui_Time22Desc && desc_21) {
        lv_label_set_text(ui_Time22Desc, desc_21);
    }
}

void update_3day_forecast_display(float temp_day1, float temp_day2, float temp_day3,
                                 const char* desc_day1, const char* desc_day2, const char* desc_day3) {
    ESP_LOGI("UI", "Updating 3-day forecast: %.1f, %.1f, %.1f", temp_day1, temp_day2, temp_day3);
    
    if (ui_Day1Temp) {
        char temp_str[16];
        if (temp_day1 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_day1);
        }
        lv_label_set_text(ui_Day1Temp, temp_str);
    }
    if (ui_Day1Desc && desc_day1) {
        lv_label_set_text(ui_Day1Desc, desc_day1);
    }
    
    if (ui_Day2Temp) {
        char temp_str[16];
        if (temp_day2 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_day2);
        }
        lv_label_set_text(ui_Day2Temp, temp_str);
    }
    if (ui_Day2Desc && desc_day2) {
        lv_label_set_text(ui_Day2Desc, desc_day2);
    }
    
    if (ui_Day3Temp) {
        char temp_str[16];
        if (temp_day3 < -50) {
            snprintf(temp_str, sizeof(temp_str), "ERR");
        } else {
            snprintf(temp_str, sizeof(temp_str), "%.0f°", temp_day3);
        }
        lv_label_set_text(ui_Day3Temp, temp_str);
    }
    if (ui_Day3Desc && desc_day3) {
        lv_label_set_text(ui_Day3Desc, desc_day3);
    }
}

void ui_update_weather_display(float temp, float feels_like, int humidity, int pressure, const char* description, const char* city) {
    ESP_LOGI("UI", "=== UPDATING WEATHER UI ===");
    
    if (ui_CurrentTempLabel) {
        char temp_str[32];
        snprintf(temp_str, sizeof(temp_str), "%.1f°C", temp);
        lv_label_set_text(ui_CurrentTempLabel, temp_str);
    }
    
    if (ui_CityLabel && city) {
        lv_label_set_text(ui_CityLabel, city);
    }
    
    if (ui_CurrentWeatherDesc && description) {
        lv_label_set_text(ui_CurrentWeatherDesc, description);
    }
    
    if (ui_FeelsLikeLabel) {
        char feels_str[32];
        snprintf(feels_str, sizeof(feels_str), "%.1f°C", feels_like);
        lv_label_set_text(ui_FeelsLikeLabel, feels_str);
    }
    
    if (ui_HumidityLabel) {
        char humidity_str[32];
        snprintf(humidity_str, sizeof(humidity_str), "%d%%", humidity);
        lv_label_set_text(ui_HumidityLabel, humidity_str);
    }
    
    if (ui_PressureLabel) {
        char pressure_str[32];
        snprintf(pressure_str, sizeof(pressure_str), "%d hPa", pressure);
        lv_label_set_text(ui_PressureLabel, pressure_str);
    }
    
    lv_task_handler();
}



void ui_notify_tts_finished(void)
{
    if (speaking_time_date) {
        unlock_speak_button();
    }
    if (speaking_weather) {
        unlock_weather_speak_buttons();
    }
    
    speaking_time_date = false;
    speaking_weather = false;
}
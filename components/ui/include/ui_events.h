#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include "lvgl.h"

void ui_event_FormatSwitch(lv_event_t * e);
void ui_event_SpeakButton(lv_event_t * e);
void ui_event_SpeakDateButton(lv_event_t * e);
void lock_speak_buttons(void);
void unlock_speak_button(void);
void update_status_display(const char* status);
void update_time_display(const char* time_str, const char* date_str);
void update_buttons_state(void);
void unlock_connect_button(void);
void unlock_update_button_async(void);
void ui_connect_to_wifi(void);
void ui_set_wifi_credentials(const char* ssid, const char* password);
void ui_event_Connect(lv_event_t * e);
void ui_notify_tts_finished(void);
void lock_weather_buttons(void);
void unlock_weather_speak_buttons(void);
void unlock_current_weather_button(void);
void unlock_forecast_buttons(void);
void ui_event_SpeakWeatherBtn(lv_event_t * e);
void ui_event_SpeakTodayBtn(lv_event_t * e);
void ui_event_SpeakForecastBtn(lv_event_t * e);
void ui_event_UpdateWeatherBtn(lv_event_t * e);

void ui_update_weather_display(float temp, float feels_like, int humidity, int pressure, const char* description, const char* city);
void update_weather_status(const char* status, bool is_error);
void update_today_forecast_display(float temp_12, float temp_17, float temp_22, 
                                  const char* desc_12, const char* desc_17, const char* desc_22);
void update_3day_forecast_display(float temp_day1, float temp_day2, float temp_day3,
                                 const char* desc_day1, const char* desc_day2, const char* desc_day3);

#endif
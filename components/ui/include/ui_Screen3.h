#ifndef UI_SCREEN3_H
#define UI_SCREEN3_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ui_Screen3_screen_init(void);
extern void ui_Screen3_screen_destroy(void);
extern void ui_event_Screen3(lv_event_t * e);
extern lv_obj_t * ui_Screen3;
extern bool weather_available;
extern lv_obj_t * ui_LeftNavBtn;
extern lv_obj_t * ui_RightNavBtn;
extern lv_obj_t * ui_CurrentTempLabel;
extern lv_obj_t * ui_CurrentWeatherDesc;  
extern lv_obj_t * ui_FeelsLikeLabel;
extern lv_obj_t * ui_HumidityLabel;
extern lv_obj_t * ui_PressureLabel;
extern lv_obj_t * ui_CityLabel;
extern lv_obj_t * ui_WeatherStatusLabel;
extern lv_obj_t * ui_SpeakWeatherBtn;
extern lv_obj_t * ui_SpeakTodayBtn;
extern lv_obj_t * ui_SpeakForecastBtn;
extern lv_obj_t * ui_TimeSlot12;
extern lv_obj_t * ui_TimeSlot17;
extern lv_obj_t * ui_TimeSlot22;
extern lv_obj_t * ui_Day1Forecast;
extern lv_obj_t * ui_Day2Forecast;
extern lv_obj_t * ui_Day3Forecast;
extern lv_obj_t * ui_NavDots_S3;
extern lv_obj_t * ui_NavDot1_S3;
extern lv_obj_t * ui_NavDot2_S3;
extern lv_obj_t * ui_NavDot3_S3;
extern lv_obj_t * ui_LeftNavBtn;
extern lv_obj_t * ui_RightNavBtn;
extern lv_obj_t * ui_UpdateWeatherBtn;

extern void ui_event_LeftNavBtn(lv_event_t * e);
extern void ui_event_RightNavBtn(lv_event_t * e);
extern void ui_event_SpeakWeatherBtn(lv_event_t * e);
extern void ui_event_SpeakTodayBtn(lv_event_t * e);
extern void ui_event_SpeakForecastBtn(lv_event_t * e);
extern void ui_event_UpdateWeatherBtn (lv_event_t * e);
extern void ui_update_weather_display(float temp, float feels_like, int humidity, int pressure, const char* description, const char* city);
extern void update_weather_status(const char* status, bool is_error);
extern void update_weather_status_with_progress(const char* status, int step, int total_steps);
extern void lock_weather_buttons(void);
extern void unlock_weather_buttons(void);

#ifdef __cplusplus
} 
#endif

#endif
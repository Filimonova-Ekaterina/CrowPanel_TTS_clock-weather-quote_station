#ifndef WEATHER_SERVICE_H
#define WEATHER_SERVICE_H

#include <stdbool.h>

#ifndef OPENWEATHER_API_KEY
#define OPENWEATHER_API_KEY "4912a2a9e8c4c1b674f7baad849b5e2a"
#endif

typedef struct {
    float temp_current;
    float temp_feels_like;
    float temp_min;
    float temp_max;
    int humidity;
    int pressure;
    int weather_code;
    char description[64];
    char city[64];
} weather_data_t;

typedef struct {
    char time[16]; 
    float temp;
    char description[64];
    int weather_code;
} forecast_slot_t;

typedef struct {
    char date[16];  
    float temp_day;
    float temp_night;
    float temp_min;
    float temp_max;
    int humidity;
    int pressure;
    char description[64];
    int weather_code;
} daily_forecast_t;

typedef struct {
    forecast_slot_t today_slots[3]; 
    daily_forecast_t daily[3]; 
    int today_slots_count;
    int daily_count;
} weather_forecast_t;

#ifdef __cplusplus
extern "C" {
#endif

bool get_current_weather(const char* city, weather_data_t* weather);
bool get_weather_by_location(weather_data_t* weather);
void weather_service_init(void);
bool is_weather_api_configured(void);

bool get_weather_forecast(const char* city, weather_forecast_t* forecast);
bool get_forecast_by_location(weather_forecast_t* forecast);

void update_weather_status(const char* status, bool is_error);
void ui_update_weather_display(float temp, float feels_like, int humidity, int pressure, const char* description, const char* city);
const char* normalize_city_name(const char* raw_city);
#ifdef __cplusplus
}
#endif

#endif
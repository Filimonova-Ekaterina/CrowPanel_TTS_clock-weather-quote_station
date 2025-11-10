#ifndef LOCATION_SERVICE_H
#define LOCATION_SERVICE_H

#include <stdbool.h>

typedef struct {
    char city[64];
    char country[64];
    char region[64];
    char timezone[64];
    double latitude;
    double longitude;
} location_data_t;

extern "C" {
    void update_debug_info(const char* info);
    void update_weather_status(const char* status, bool is_error);
    void update_weather_status_with_progress(const char* status, int step, int total_steps);
}
bool get_location_data(location_data_t* location);
const char* get_city_name(void);
const char* get_timezone_by_location(void);
void location_service_init(void);
bool set_manual_location(const char* city);
bool clear_manual_location(void);

#ifdef __cplusplus
extern "C" {
#endif

void update_weather_status(const char* status, bool is_error);
void update_weather_status_with_progress(const char* status, int step, int total_steps);

#ifdef __cplusplus
}
#endif

#endif
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "nvs.h"

#include "esp_display_panel.hpp"
#include "lvgl_v8_port.h"
#include "HxTTS.h"
#include "uart_manager.h"
#include "uart_json.h"
#include "ui.h"
#include "ui_events.h"
#include "tts_bridge.h"
#include "time_formatter.h"
#include "timezone_service.h"
#include "weather_service.h"
#include "location_service.h"
#include "quote_service.h"

#define MAX_RETRY 3
#define WIFI_RETRY_DELAY_MS 2000
using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char* TAG = "clock_tts";

HxTTS *g_hx_tts = nullptr;
TimezoneService tz_service;
TimeFormatter time_formatter;

char current_time_str[64] = "00:00:00";
char current_date_str[64] = "2025-01-01";
char status_str[128] = "Starting...";
static char current_quote[1024] = {0};
static char current_author[128] = {0};
static bool quote_available = false;

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAILED_BIT    = BIT1;
const int TIME_SYNCED_BIT    = BIT2;

typedef enum {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_FAILED
} wifi_state_t;

static wifi_state_t current_wifi_state = WIFI_STATE_DISCONNECTED;
static SemaphoreHandle_t wifi_state_mutex = NULL;
extern lv_obj_t * ui_SpeakWeatherBtn;
extern lv_obj_t * ui_SpeakTodayBtn;
extern lv_obj_t * ui_SpeakForecastBtn;
int retry_num = 0;
bool wifi_connected = false;
static bool weather_update_requested = false;
static SemaphoreHandle_t weather_update_mutex = NULL;
static bool time_synced = false;
static bool display_initialized = false;
static bool sntp_initialized = false;
static bool system_initialized = false;
bool time_initialized = false; 
static bool first_boot = true;
static bool time_initialization_in_progress = false;
static bool wifi_initialized = false;
bool wifi_connection_in_progress = false;
weather_data_t current_weather = {0};
bool weather_available = false;
static TaskHandle_t timezone_task_handle = NULL;
static bool timezone_detection_pending = false;
static SemaphoreHandle_t weather_mutex = NULL;
weather_forecast_t current_forecast = {0};
bool forecast_available = false;
static bool wifi_connection_active = false;
static SemaphoreHandle_t quote_mutex = NULL;
static bool quote_update_requested = false;
static bool initial_quote_loaded = false;

static void set_status(const char* status);
static void reconnect_wifi_delayed(void *arg);
static void switch_to_main_screen_delayed(void *arg);
static void switch_to_wifi_setup_screen_delayed(void *arg);
static void cleanup_wifi_connection(void);
static bool start_wifi_connection(const char* ssid, const char* password);
static void set_wifi_state(wifi_state_t new_state);
static wifi_state_t get_wifi_state(void);
static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data);

extern "C" {
    void update_time_display(const char* time_str, const char* date_str);
    void update_status_display(const char* status);
    void ui_notify_tts_finished(void);
    void unlock_speak_button(void);
    void unlock_forecast_buttons(void);
    void unlock_current_weather_button(void);
    void unlock_update_button_async(void);
    void lock_speak_buttons(void);
    void update_time(void);
    void ui_update_weather_display(float temp, float feels_like, int humidity, int pressure, const char* description, const char* city);
    void update_analog_clock(int hours, int minutes, int seconds);
    void time_formatter_set_12h_format(bool enabled) {
        time_formatter.set_12h_format(enabled);
    }
    const char* normalize_city_name(const char* raw_city);
    const char* format_time_for_display_c(int hours, int minutes, int seconds) {
        return time_formatter.format_time_display_c(hours, minutes, seconds);
    }
    const char* format_time_for_tts_c(int hours, int minutes) {
        return time_formatter.format_time_for_tts_c(hours, minutes);
    }
    void update_today_forecast_display(float temp_12, float temp_17, float temp_22, 
                                      const char* desc_12, const char* desc_17, const char* desc_22);
    void update_3day_forecast_display(float temp_day1, float temp_day2, float temp_day3,
                                     const char* desc_day1, const char* desc_day2, const char* desc_day3);
    void update_quote_text(const char* quote_text);
    void update_quote_status(const char* status, bool is_error);
    void unlock_quote_buttons(void);
    bool get_random_quote_simple(char* quote_buffer, size_t buffer_size);
}


extern "C" void update_weather_data(void) {
    ESP_LOGI("WEATHER", "=== UPDATE WEATHER DATA CALLED ===");
    if (ui_SpeakWeatherBtn) {
        lv_obj_add_state(ui_SpeakWeatherBtn, LV_STATE_DISABLED);
    }
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    memset(&current_weather, 0, sizeof(weather_data_t));
    strcpy(current_weather.city, "Unknown");
    strcpy(current_weather.description, "Unknown");
    current_weather.temp_current = -100; 
    if (weather_mutex) xSemaphoreGive(weather_mutex);
    
    weather_data_t weather;
    memset(&weather, 0, sizeof(weather_data_t)); 
    
    const char* city = get_city_name();
    city = normalize_city_name(city);
    char status_msg[64];
    snprintf(status_msg, sizeof(status_msg), "Req: %.20s", city);
    update_weather_status(status_msg, false);
    
    lv_task_handler();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    if (get_weather_by_location(&weather)) {
        ESP_LOGI("WEATHER", "RAW DATA: %s, %.1fC", weather.city, weather.temp_current);
        
        if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
        memcpy(&current_weather, &weather, sizeof(weather_data_t));
        weather_available = true;
        if (weather_mutex) xSemaphoreGive(weather_mutex);

        weather_data_t ui_copy;
        if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
        memcpy(&ui_copy, &current_weather, sizeof(weather_data_t));
        if (weather_mutex) xSemaphoreGive(weather_mutex);

        ui_update_weather_display(
            ui_copy.temp_current,
            ui_copy.temp_feels_like,
            ui_copy.humidity,
            ui_copy.pressure,
            ui_copy.description,
            ui_copy.city
        );
        
        unlock_current_weather_button();
    } else {
        ESP_LOGW("WEATHER", "Failed to get weather data");
        weather_available = false;
        update_weather_status("Update failed", true);
    }
    if (wifi_connected){
    unlock_update_button_async();
    }
    
}

static void safe_async_call(void (*func)(void *), void *arg) {
   
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        if (func != NULL) {
            lv_async_call(func, arg);
        }
    } else {
        ESP_LOGW(TAG, "Scheduler not ready, delaying async call");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (func != NULL) {
            func(arg);
        }
    }
}
extern "C" void update_forecast_data(void) {
    ESP_LOGI("FORECAST", "=== FORECAST UPDATE START ===");
    if (ui_SpeakTodayBtn) {
        lv_obj_add_state(ui_SpeakTodayBtn, LV_STATE_DISABLED);
    }
    if (ui_SpeakForecastBtn) {
        lv_obj_add_state(ui_SpeakForecastBtn, LV_STATE_DISABLED);
    }
    update_today_forecast_display(0, 0, 0, "LOADING...", "LOADING...", "LOADING...");
    update_3day_forecast_display(0, 0, 0, "LOADING...", "LOADING...", "LOADING...");
    
    for(int i = 0; i < 3; i++) {
        lv_task_handler();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    weather_forecast_t forecast;
    memset(&forecast, 0, sizeof(weather_forecast_t));
    
    bool success = get_forecast_by_location(&forecast);
    
    if (!success) {
        ESP_LOGE("FORECAST", "API call failed");
        return;
    }
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    memcpy(&current_forecast, &forecast, sizeof(weather_forecast_t));
    forecast_available = true;
    if (weather_mutex) xSemaphoreGive(weather_mutex);
    
    ESP_LOGI("FORECAST", "Forecast saved: %d today slots, %d daily", 
             forecast.today_slots_count, forecast.daily_count);
    
    if (forecast.today_slots_count >= 3) {
        update_today_forecast_display(
            forecast.today_slots[0].temp,
            forecast.today_slots[1].temp,  
            forecast.today_slots[2].temp,
            forecast.today_slots[0].description,
            forecast.today_slots[1].description,
            forecast.today_slots[2].description
        );
        unlock_forecast_buttons();
        update_weather_status("Forecast: TODAY OK", false);
    } else {
        update_weather_status("Forecast: TODAY partial", true);
    }
    
    if (forecast.daily_count >= 3) {
        update_3day_forecast_display(
            forecast.daily[0].temp_day,
            forecast.daily[1].temp_day,
            forecast.daily[2].temp_day,
            forecast.daily[0].description,
            forecast.daily[1].description, 
            forecast.daily[2].description
        );
        unlock_forecast_buttons();
    } else if (forecast.daily_count > 0) {
        float temp1 = forecast.daily_count > 0 ? forecast.daily[0].temp_day : 0;
        float temp2 = forecast.daily_count > 1 ? forecast.daily[1].temp_day : 0;
        float temp3 = forecast.daily_count > 2 ? forecast.daily[2].temp_day : 0;
        
        const char* desc1 = forecast.daily_count > 0 ? forecast.daily[0].description : "N/A";
        const char* desc2 = forecast.daily_count > 1 ? forecast.daily[1].description : "N/A";
        const char* desc3 = forecast.daily_count > 2 ? forecast.daily[2].description : "N/A";
        
        update_3day_forecast_display(temp1, temp2, temp3, desc1, desc2, desc3);
        unlock_forecast_buttons();
    } else {
        update_3day_forecast_display(0, 0, 0, "NO DATA", "NO DATA", "NO DATA");
    }
    if (wifi_connected){
    unlock_update_button_async();
    }
    update_weather_status("Done! Type for update again.", false);
    ESP_LOGI("FORECAST", "=== FORECAST UPDATE COMPLETE ===");
    
}

extern "C" void speak_current_weather(void) {
    if (weather_available) {
        weather_data_t weather_copy;
        if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
        memcpy(&weather_copy, &current_weather, sizeof(weather_data_t));
        if (weather_mutex) xSemaphoreGive(weather_mutex);

        char phrase[1024];
        snprintf(phrase, sizeof(phrase),
                 "Current weather in %s: %.1f degrees, %s. Feels like %.1f degrees. Humidity is %d percent and pressure is %d H P A.",
                 weather_copy.city,
                 weather_copy.temp_current, 
                 weather_copy.description,
                 weather_copy.temp_feels_like,
                 weather_copy.humidity,
                 weather_copy.pressure);
        
        if (g_hx_tts) {
            start_tts_playback_c(phrase);
        }
    } else {
        if (g_hx_tts) {
            start_tts_playback_c("Weather data is not available yet.");
        }
    }
}

extern "C" void speak_today_forecast(void) {
    ESP_LOGI("TTS", "speak_today_forecast called");
    
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    bool has_weather = weather_available;
    bool has_forecast = forecast_available;
    int today_slots = current_forecast.today_slots_count;
    if (weather_mutex) xSemaphoreGive(weather_mutex);
    
    ESP_LOGI("TTS", "Diagnostic: weather=%d, forecast=%d, today_slots=%d", 
             has_weather, has_forecast, today_slots);
    
    if (has_weather && has_forecast && today_slots >= 3) {
        weather_data_t weather_copy;
        weather_forecast_t forecast_copy;
        
        if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
        memcpy(&weather_copy, &current_weather, sizeof(weather_data_t));
        memcpy(&forecast_copy, &current_forecast, sizeof(weather_forecast_t));
        if (weather_mutex) xSemaphoreGive(weather_mutex);

        char phrase[1024];
        char time_9[64], time_15[64], time_21[64];
        snprintf(time_9, sizeof(time_9), "%s", forecast_copy.today_slots[0].time);
        snprintf(time_15, sizeof(time_15), "%s", forecast_copy.today_slots[1].time);  
        snprintf(time_21, sizeof(time_21), "%s", forecast_copy.today_slots[2].time);

        time_9[strcspn(time_9, ":")] = '\0';
        time_15[strcspn(time_15, ":")] = '\0';
        time_21[strcspn(time_21, ":")] = '\0';

        snprintf(phrase, sizeof(phrase),
                "Today's forecast for %s. "
                "Morning at %s oclock: %.0f degrees and %s. "
                "Afternoon at %s oclock: %.0f degrees and %s. "
                "Evening at %s oclock: %.0f degrees and %s.",
                weather_copy.city,
                time_9, forecast_copy.today_slots[0].temp, 
                forecast_copy.today_slots[0].description,
                time_15, forecast_copy.today_slots[1].temp, 
                forecast_copy.today_slots[1].description,
                time_21, forecast_copy.today_slots[2].temp, 
                forecast_copy.today_slots[2].description);
        if (g_hx_tts) {
            start_tts_playback_c(phrase);
        }
    } else {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), 
                 "Today forecast not available. Weather: %d, Forecast: %d, Slots: %d", 
                 has_weather, has_forecast, today_slots);
        ESP_LOGW("TTS", "%s", error_msg);
        
        if (g_hx_tts) {
            start_tts_playback_c("Today's forecast is not available yet. Please wait for data update.");
        }
    }
}

extern "C" void request_weather_update(void) {
    if (weather_update_mutex) xSemaphoreTake(weather_update_mutex, portMAX_DELAY);
    weather_update_requested = true;
    if (weather_update_mutex) xSemaphoreGive(weather_update_mutex);
    

    ESP_LOGI(TAG, "Weather update requested from UI");
}

extern "C" void speak_days_forecast(int days) {
    ESP_LOGI("TTS", "speak_days_forecast called for %d days", days);
    
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    bool has_weather = weather_available;
    bool has_forecast = forecast_available;
    int daily_count = current_forecast.daily_count;
    if (weather_mutex) xSemaphoreGive(weather_mutex);
    
    ESP_LOGI("TTS", "Diagnostic: weather=%d, forecast=%d, daily_count=%d", 
             has_weather, has_forecast, daily_count);
    
    if (has_weather && has_forecast && days > 0 && days <= 3) {
        weather_data_t weather_copy;
        weather_forecast_t forecast_copy;
        
        if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
        memcpy(&weather_copy, &current_weather, sizeof(weather_data_t));
        memcpy(&forecast_copy, &current_forecast, sizeof(weather_forecast_t));
        if (weather_mutex) xSemaphoreGive(weather_mutex);

        if (forecast_copy.daily_count >= days) {
            char phrase[512];
            snprintf(phrase, sizeof(phrase), "%d day forecast for %s   : ", days, weather_copy.city);
            
            for (int i = 0; i < days && i < forecast_copy.daily_count; i++) {
                daily_forecast_t* day = &forecast_copy.daily[i];
                char day_str[128];
                
                char spoken_date[32];
                if (i == 0) {
                    strcpy(spoken_date, "Tomorrow");
                } else if (i == 1) {
                    strcpy(spoken_date, "Day after tommorow");
                } else if (i == 2) {
                    strcpy(spoken_date, "In two days");
                }
                
                snprintf(day_str, sizeof(day_str),
                         "%s   %.0f     degrees,    %s.   ",
                         spoken_date, day->temp_day, day->description);
                
                strlcat(phrase, day_str, sizeof(phrase));
            }
            
            ESP_LOGI("TTS", "Days forecast phrase: %s", phrase);
            
            if (g_hx_tts) {
                start_tts_playback_c(phrase);
            }
        } else {
            char msg[128];
            snprintf(msg, sizeof(msg), 
                     "Only %d days of forecast available. Requested %d days.", 
                     forecast_copy.daily_count, days);
            ESP_LOGW("TTS", "%s", msg);
            
            if (g_hx_tts) {
                start_tts_playback_c(msg);
            }
        }
    } else {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg),
                 "Forecast not available. Weather: %d, Forecast: %d, Requested: %d days",
                 has_weather, has_forecast, days);
        ESP_LOGW("TTS", "%s", error_msg);
        
        if (g_hx_tts) {
            start_tts_playback_c("Forecast data is not available yet. Please wait for data update.");
        }
    }
}

static void set_wifi_state(wifi_state_t new_state) {
    if (wifi_state_mutex) {
        xSemaphoreTake(wifi_state_mutex, portMAX_DELAY);
    }
    
    wifi_state_t old_state = current_wifi_state;
    current_wifi_state = new_state;
    
    ESP_LOGI(TAG, "WiFi state changed: %d -> %d", old_state, new_state);
    
    if (wifi_state_mutex) {
        xSemaphoreGive(wifi_state_mutex);
    }
}

static wifi_state_t get_wifi_state(void) {
    wifi_state_t state;
    if (wifi_state_mutex) {
        xSemaphoreTake(wifi_state_mutex, portMAX_DELAY);
    }
    state = current_wifi_state;
    if (wifi_state_mutex) {
        xSemaphoreGive(wifi_state_mutex);
    }
    return state;
}

static void set_default_time(void) {
    struct timeval tv = { .tv_sec = 1735689600, .tv_usec = 0 };
    if (settimeofday(&tv, NULL) == 0)
        ESP_LOGI(TAG, "Default time set");
    else
        ESP_LOGE(TAG, "Failed to set default time");
}

static bool get_safe_time(struct tm* timeinfo) {
    time_t now;
    time(&now);
    if (localtime_r(&now, timeinfo) == NULL) {
        ESP_LOGE(TAG, "Failed to get local time, setting default");
        set_default_time();
        time(&now);
        if (localtime_r(&now, timeinfo) == NULL) {
            timeinfo->tm_year = 125; 
            timeinfo->tm_mon = 0;
            timeinfo->tm_mday = 1;
            timeinfo->tm_hour = 0;
            timeinfo->tm_min = 0;
            timeinfo->tm_sec = 0;
            return false;
        }
    }
    return true;
}

static void set_status(const char* status) {
    char* dest = status_str;
    const char* src = status;
    int i = 0;
    while (*src && i < sizeof(status_str) - 1) {
        *dest++ = *src++;
        i++;
    }
    *dest = '\0';
    
    if (display_initialized) update_status_display(status_str);
    ESP_LOGI(TAG, "%s", status);
}

static void time_sync_notification_cb(struct timeval *tv) {
    time_synced = true;
    xEventGroupSetBits(wifi_event_group, TIME_SYNCED_BIT);
    struct tm t;
    localtime_r(&tv->tv_sec, &t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    ESP_LOGI(TAG, "Time synchronized: %s", buf);
}

static void initialize_sntp(void) {
    if (sntp_initialized) {
        ESP_LOGI(TAG, "Stopping previous SNTP");
        esp_sntp_stop();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.windows.com");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_set_sync_interval(60000);
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    esp_sntp_init();
    sntp_initialized = true;
    ESP_LOGI(TAG, "SNTP initialized");
}


extern "C" void update_wifi_connection_status(const char* status, int retry_num, int max_retry) {
    if (ui_StatusLabel) {
        lv_label_set_text(ui_StatusLabel, status);
    }
    
    if (ui_Label3 && retry_num > 0) {
        char button_text[32];
        snprintf(button_text, sizeof(button_text), "RETRY %d/%d", retry_num, max_retry);
        lv_label_set_text(ui_Label3, button_text);
    }
    
    lv_task_handler();
}

static void timezone_detection_task(void* arg) {
    ESP_LOGI(TAG, "Timezone detection task started");
    
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Detecting timezone...");
    std::string detected_tz = tz_service.detect_timezone();
    
    if (!detected_tz.empty()) {
        setenv("TZ", detected_tz.c_str(), 1);
        tzset();
        ESP_LOGI(TAG, "Timezone set to: %s", detected_tz.c_str());
        set_status("Time synchronized!");
    } else {
        ESP_LOGW(TAG, "Timezone detection failed, using GMT");
        set_status("Time synced (GMT)");
    }
    
    timezone_detection_pending = false;
    timezone_task_handle = NULL;
    vTaskDelete(NULL);
}

static void initialize_time(void) {
    if (time_initialization_in_progress) {
        ESP_LOGW(TAG, "Time initialization already in progress");
        return;
    }
    
    time_initialization_in_progress = true;
    set_status("Initializing time...");
    lock_speak_buttons();
    setenv("TZ", "GMT0", 1);
    tzset();
    set_default_time();
    update_time();
    struct tm t;
    if (get_safe_time(&t)) {
        time_initialized = true;
        ESP_LOGI(TAG, "Basic time initialized");
    }
    
    unlock_speak_button();

    if (wifi_connected) {
        lock_speak_buttons();
        ESP_LOGI(TAG, "Starting time synchronization...");
        
        time_synced = false;
        if (wifi_event_group) {
            xEventGroupClearBits(wifi_event_group, TIME_SYNCED_BIT);
        }
        
        initialize_sntp();
        
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group, TIME_SYNCED_BIT, 
                                              pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
        
        if (bits & TIME_SYNCED_BIT) {
            ESP_LOGI(TAG, "Time sync completed successfully");
            if (!timezone_detection_pending && timezone_task_handle == NULL) {
                timezone_detection_pending = true;
                BaseType_t result = xTaskCreate(
                    timezone_detection_task,
                    "tz_detect",
                    8192,
                    NULL,
                    2,
                    &timezone_task_handle
                );
                
                if (result == pdPASS) {
                    ESP_LOGI(TAG, "Timezone detection task started");
                    set_status("Detecting timezone...");
                } else {
                    ESP_LOGE(TAG, "Failed to create timezone detection task");
                    timezone_detection_pending = false;
                    set_status("Time synced (GMT)");
                }
            }
        } else {
            ESP_LOGW(TAG, "Time synchronization timeout");
            set_status("Time sync failed");
        }
        unlock_speak_button();
    } else {
        ESP_LOGI(TAG, "Using system time without sync");
        set_status("System time ready");
        unlock_speak_button();
    }

    system_initialized = true;
    time_initialization_in_progress = false;
}

static void initialize_wifi_system(void) {
    if (wifi_initialized) {
        ESP_LOGI(TAG, "WiFi system already initialized");
        return;
    }
    ESP_LOGI(TAG, "Initializing WiFi system...");
    wifi_state_mutex = xSemaphoreCreateMutex();
    if (wifi_state_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi state mutex");
        return;
    }
    
    if (wifi_event_group == NULL) {
        wifi_event_group = xEventGroupCreate();
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    wifi_initialized = true;
    set_wifi_state(WIFI_STATE_DISCONNECTED);
    ESP_LOGI(TAG, "WiFi system initialized successfully");
}

static void cleanup_wifi_connection(void) {
    ESP_LOGI(TAG, "Cleaning up WiFi connection...");
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    esp_wifi_disconnect();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    wifi_connected = false;
    time_synced = false;
    retry_num = 0;
    wifi_connection_active = false;
    set_wifi_state(WIFI_STATE_DISCONNECTED);
    if (weather_mutex) xSemaphoreGive(weather_mutex);

    ESP_LOGI(TAG, "WiFi connection cleaned up");
}

static bool start_wifi_connection(const char* ssid, const char* password) {
    if (get_wifi_state() == WIFI_STATE_CONNECTING) {
        ESP_LOGW(TAG, "WiFi connection already in progress");
        return false;
    }

    cleanup_wifi_connection();
    
    if (!ssid || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid SSID provided");
        return false;
    }

    ESP_LOGI(TAG, "Starting WiFi connection to: %s", ssid);
    
    if (!wifi_initialized) {
        initialize_wifi_system();
    }

    wifi_config_t wifi_config = {};
    strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (password) {
        strlcpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    wifi_connection_in_progress = true;
    wifi_connection_active = true;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    set_wifi_state(WIFI_STATE_CONNECTING);
    
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "WiFi connection process started");
    return true;
}

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data) {
    if (!wifi_event_group) return;
    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    if (base == WIFI_EVENT) {
        switch (id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                if (get_wifi_state() == WIFI_STATE_CONNECTING) {
                    esp_wifi_connect();
                }
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) data;
                if (!wifi_connection_in_progress) {
                    wifi_connection_active = false;}
                wifi_connected = false;
                time_synced = false;
                set_wifi_state(WIFI_STATE_DISCONNECTED);
                
                if (wifi_connection_in_progress && retry_num < MAX_RETRY) {
                    retry_num++;
                    
                    const char* reason = "Unknown";
                    switch(event->reason) {
                        case WIFI_REASON_AUTH_FAIL: reason = "Wrong password"; break;
                        case WIFI_REASON_NO_AP_FOUND: reason = "Network not found"; break;
                        case WIFI_REASON_ASSOC_FAIL: reason = "Association failed"; break;
                        case WIFI_REASON_AUTH_EXPIRE: reason = "Auth expired"; break;
                        case WIFI_REASON_CONNECTION_FAIL: reason = "Connection failed"; break;
                    }
                    
                    char status_msg[128];
                    snprintf(status_msg, sizeof(status_msg), 
                            "%s - Retry %d/%d", reason, retry_num, MAX_RETRY);
                    set_status(status_msg);
                    
                    update_wifi_connection_status(status_msg, retry_num, MAX_RETRY);
                    
                    ESP_LOGW(TAG, "WiFi disconnected (%s), retrying %d/%d", 
                            reason, retry_num, MAX_RETRY);
                    
                    safe_async_call(reconnect_wifi_delayed, NULL);
                } else {
                    wifi_connection_in_progress = false;
                    
                    const char* final_reason = "Connection failed";
                    if (event->reason == WIFI_REASON_AUTH_FAIL) {
                        final_reason = "Wrong password";
                    } else if (event->reason == WIFI_REASON_NO_AP_FOUND) {
                        final_reason = "Network not found";
                    }
                    
                    char final_msg[64];
                    snprintf(final_msg, sizeof(final_msg), "FAILED: %s", final_reason);
                    set_status(final_msg);
                    
                    update_wifi_connection_status("FAILED - Enter new credentials", 0, 0);
                    if (ui_Label3) {
                        lv_label_set_text(ui_Label3, "FAILED");
                    }
                    
                    if (get_wifi_state() == WIFI_STATE_CONNECTING) {
                        ESP_LOGI(TAG, "Auto-connect failed, switching to WiFi setup screen");
                        safe_async_call(switch_to_wifi_setup_screen_delayed, NULL);
                    }
                    
                    set_wifi_state(WIFI_STATE_FAILED);
                    ESP_LOGE(TAG, "WiFi connection failed after %d attempts", MAX_RETRY);
                }
                break;
            }
            
            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "WiFi STA stopped");
                set_wifi_state(WIFI_STATE_DISCONNECTED);
                wifi_connection_in_progress = false;
                break;
        }
    } 
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) data;
        wifi_connection_active = false;
        wifi_connected = true;
        wifi_connection_in_progress = false;
        retry_num = 0;
        set_wifi_state(WIFI_STATE_CONNECTED);
        
        char ip_str[32];
        snprintf(ip_str, sizeof(ip_str), "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        set_status(ip_str);
        
        ESP_LOGI(TAG, "WiFi connected successfully! %s", ip_str);
        
        if (ui_Label3) {
            lv_label_set_text(ui_Label3, "CONNECTED!");
        }
        update_wifi_connection_status("Connected! Switching...", 0, 0);
        
        safe_async_call(switch_to_main_screen_delayed, NULL);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        initialize_time();
        
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    if (weather_mutex) xSemaphoreGive(weather_mutex);
}

extern "C" void connect_to_wifi_manual(const char* ssid, const char* password) {
    ESP_LOGI(TAG, "=== MANUAL WIFI CONNECTION REQUEST ===");
    
    if (!ssid || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid SSID provided");
        unlock_connect_button(); 
        return;
    }
    extern char wifi_ssid[64];
    extern char wifi_password[64];
    strlcpy(wifi_ssid, ssid, sizeof(wifi_ssid));
    if (password) {
        strlcpy(wifi_password, password, sizeof(wifi_password));
    }
    if (start_wifi_connection(ssid, password)) {
        ESP_LOGI(TAG, "WiFi connection process started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start WiFi connection");
        unlock_connect_button();
    }
}

static void reconnect_wifi_delayed(void *arg) {
    vTaskDelay(WIFI_RETRY_DELAY_MS / portTICK_PERIOD_MS);
       
    if (!wifi_initialized || !wifi_event_group) {
        return;
    }
    if (get_wifi_state() == WIFI_STATE_DISCONNECTED && 
        wifi_connection_in_progress && 
        retry_num <= MAX_RETRY) {
        
        ESP_LOGI(TAG, "Attempting WiFi reconnection %d/%d", retry_num, MAX_RETRY);
        vTaskDelay(500 / portTICK_PERIOD_MS);        
        char retry_msg[64];
        snprintf(retry_msg, sizeof(retry_msg), "Retrying %d/%d...", retry_num, MAX_RETRY);
        update_wifi_connection_status(retry_msg, retry_num, MAX_RETRY);
        
        esp_wifi_connect();
    }
}

static void switch_to_wifi_setup_screen_delayed(void *arg) {
    for (int i = 0; i < 5; i++) {
        lv_task_handler();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
    unlock_connect_button();
}

static void switch_to_main_screen_delayed(void *arg) {

    for (int i = 0; i < 5; i++) {
        lv_task_handler();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    
    _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen1_screen_init);
}

extern "C" void update_time(void) {
    struct tm t;
    if (!get_safe_time(&t)) {
        update_time_display("--:--:--", "2025-01-01");
        return;
    }

    const char* formatted = format_time_for_display_c(t.tm_hour, t.tm_min, t.tm_sec);
    
    char date_buf[32];
    snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", 
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    
    char* dest = current_date_str;
    char* src = date_buf;
    int i = 0;
    while (*src && i < sizeof(current_date_str) - 1) {
        *dest++ = *src++;
        i++;
    }
    *dest = '\0';
    
    update_analog_clock(t.tm_hour, t.tm_min, t.tm_sec);
    update_time_display(formatted, current_date_str);
}

extern "C" void speak_current_time(void) {
    if (!g_hx_tts) return;
    struct tm t; 
    if (get_safe_time(&t))
        start_tts_playback_c(format_time_for_tts_c(t.tm_hour, t.tm_min));
}

static const char* get_day_with_ordinal(int day) {
    static char buf[32];

    const char* ones[] = {
        "", "first", "second", "third", "fourth", "fifth",
        "sixth", "seventh", "eighth", "ninth"
    };

    const char* teens[] = {
        "tenth", "eleventh", "twelfth", "thirteenth", "fourteenth",
        "fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth"
    };

    const char* tens_prefix[] = {
        "", "", "twenty", "thirty"
    };

    if (day <= 0 || day > 31) {
        snprintf(buf, sizeof(buf), "%d", day);
        return buf;
    }

    if (day < 10) {
        snprintf(buf, sizeof(buf), "%s", ones[day]);
    } else if (day < 20) {
        snprintf(buf, sizeof(buf), "%s", teens[day - 10]);
    } else if (day % 10 == 0) {
        if (day == 20) snprintf(buf, sizeof(buf), "twentieth");
        else if (day == 30) snprintf(buf, sizeof(buf), "thirtieth");
        else snprintf(buf, sizeof(buf), "%d", day);
    } else {
        int t = day / 10;
        int o = day % 10;
        snprintf(buf, sizeof(buf), "%s %s", tens_prefix[t], ones[o]);
    }

    return buf;
}

extern "C" void speak_current_date(void) {
    if (!g_hx_tts) return;
    struct tm t;
    if (!get_safe_time(&t)) return;
    
    const char* months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    
    char phrase[128];
    snprintf(phrase, sizeof(phrase), "Today is %s %s, %d", 
             months[t.tm_mon], get_day_with_ordinal(t.tm_mday), t.tm_year + 1900);
    
    start_tts_playback_c(phrase);
}

static void weather_update_task(void* arg) {
    ESP_LOGI(TAG, "Weather update task started - waiting for requests");
    
    if (weather_update_mutex == NULL) {
        weather_update_mutex = xSemaphoreCreateMutex();
    }
    
    while (!wifi_connected) {
        update_weather_status("Waiting for WiFi...", false);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
    ESP_LOGI(TAG, "WiFi connected - doing initial weather update");
    update_weather_data();
    update_forecast_data();
    update_weather_status("Type for update", false);
    
    while (true) {
        bool should_update = false;
        
        if (weather_update_mutex) xSemaphoreTake(weather_update_mutex, portMAX_DELAY);
        should_update = weather_update_requested;
        weather_update_requested = false; 
        if (weather_update_mutex) xSemaphoreGive(weather_update_mutex);
        
        if (should_update) {
            ESP_LOGI(TAG, "Processing weather update request");
            update_weather_data();
            update_forecast_data();
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}


extern "C" void update_quote_data(void) {
    ESP_LOGI("QUOTE", "=== UPDATE QUOTE DATA CALLED ===");
    
    update_quote_status("Fetching quote...", false);
    
    char quote_text[1024] = {0};
    
    if (get_random_quote_simple(quote_text, sizeof(quote_text))) {
        ESP_LOGI("QUOTE", "Successfully got quote: %s", quote_text);
        
        if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
        strlcpy(current_quote, quote_text, sizeof(current_quote));
        
        char* author_start = strrchr(quote_text, '-');
        if (author_start) {
            strlcpy(current_author, author_start + 2, sizeof(current_author));
        } else {
            strlcpy(current_author, "Unknown", sizeof(current_author));
        }
        
        quote_available = true;
        initial_quote_loaded = true;
        if (quote_mutex) xSemaphoreGive(quote_mutex);
        update_quote_text(quote_text);
        update_quote_status("Done! Update for new quote", false);
        
    } else {
        ESP_LOGW("QUOTE", "Failed to get quote from API");
        update_quote_status("Update failed", true);
    }
    unlock_quote_buttons();
}

extern "C" void speak_current_quote(void) {
    ESP_LOGI("QUOTE_TTS", "speak_current_quote called - using displayed quote");
    
    if (!g_hx_tts) {
        ESP_LOGE("QUOTE_TTS", "TTS not initialized");
        return;
    }
    char quote_copy[1024] = {0};
    bool quote_is_available = false;
    if (quote_mutex) {
        xSemaphoreTake(quote_mutex, portMAX_DELAY);
    }
    
    if (quote_available && strlen(current_quote) > 0) {
        strlcpy(quote_copy, current_quote, sizeof(quote_copy));
        quote_is_available = true;
        ESP_LOGI("QUOTE_TTS", "Using displayed quote: %.100s", current_quote);
    } else {
        ESP_LOGW("QUOTE_TTS", "No displayed quote available - this is an error state");
    }
    
    if (quote_mutex) {
        xSemaphoreGive(quote_mutex);
    }
    
    if (quote_is_available) {
        char tts_phrase[1200] = {0};
        
        char* separator = strstr(quote_copy, " - ");
        if (separator) {
            *separator = '\0';
            char* quote_part = quote_copy;
            char* author_part = separator + 3;
            
            if (quote_part[0] == '"' && quote_part[strlen(quote_part)-1] == '"') {
                quote_part++;
                quote_part[strlen(quote_part)-1] = '\0';
            }
            int quote_len = strlen(quote_part);
            while (quote_len > 0 && 
                  (quote_part[quote_len - 1] == '.' || 
                   quote_part[quote_len - 1] == '!' || 
                   quote_part[quote_len - 1] == '?')) {
                quote_part[quote_len - 1] = '\0';
                quote_len--;
            }
            snprintf(tts_phrase, sizeof(tts_phrase), "%s. By %s.", quote_part, author_part);
        } else {
            int quote_len = strlen(quote_copy);
            while (quote_len > 0 && 
                  (quote_copy[quote_len - 1] == '.' || 
                   quote_copy[quote_len - 1] == '!' || 
                   quote_copy[quote_len - 1] == '?')) {
                quote_copy[quote_len - 1] = '\0';
                quote_len--;
            }
            snprintf(tts_phrase, sizeof(tts_phrase), "%s.", quote_copy);
        }
        
        ESP_LOGI("QUOTE_TTS", "TTS phrase: %s", tts_phrase);
        start_tts_playback_c(tts_phrase);
        
    } else {
        ESP_LOGE("QUOTE_TTS", "No quote available to speak - this should not happen");
        start_tts_playback_c("No quote is currently available.");
    }
}

extern "C" void request_quote_update(void) {
    if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
    quote_update_requested = true;
    if (quote_mutex) xSemaphoreGive(quote_mutex);
    
    ESP_LOGI(TAG, "Quote update requested from UI");
}

static void quote_update_task(void* arg) {
    ESP_LOGI(TAG, "Quote update task started - waiting for WiFi");
    
    if (quote_mutex == NULL) {
        quote_mutex = xSemaphoreCreateMutex();
    }
    
    while (!wifi_connected) {
        update_quote_status("Waiting for WiFi...", false);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    
    ESP_LOGI(TAG, "WiFi connected - doing initial quote load");
    update_quote_data();
    
    if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
    bool is_available = quote_available;  
    if (quote_mutex) xSemaphoreGive(quote_mutex);
    
    if (is_available) {
        update_quote_status("Done! Update for new quote", false);
        ESP_LOGI(TAG, "Initial quote loaded successfully");
    } else {
        update_quote_status("Quote load failed", true);
        ESP_LOGW(TAG, "Initial quote load failed");
    }
    
    while (true) {
        bool should_update = false;
        
        if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
        should_update = quote_update_requested;
        quote_update_requested = false;
        if (quote_mutex) xSemaphoreGive(quote_mutex);
        
        if (should_update) {
            ESP_LOGI(TAG, "Processing quote update request");
            update_quote_data();
            
            if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
            bool update_success = quote_available;  
            if (quote_mutex) xSemaphoreGive(quote_mutex);
            
            if (update_success) {
                ESP_LOGI(TAG, "Quote update completed successfully");
            } else {
                ESP_LOGW(TAG, "Quote update failed");
            }
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}
static void sequential_tasks(void* arg) {
    ESP_LOGI(TAG, "Starting sequential tasks...");
    
    while (!system_initialized) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    if (weather_mutex) xSemaphoreTake(weather_mutex, portMAX_DELAY);
    memset(&current_weather, 0, sizeof(weather_data_t));
    memset(&current_forecast, 0, sizeof(weather_forecast_t));
    weather_available = false;
    forecast_available = false;
    if (weather_mutex) xSemaphoreGive(weather_mutex);
    
    update_weather_data();
    update_forecast_data(); 
    
    ESP_LOGI(TAG, "System initialized. Starting services...");

    if (quote_mutex) xSemaphoreTake(quote_mutex, portMAX_DELAY);
    memset(current_quote, 0, sizeof(current_quote));  
    memset(current_author, 0, sizeof(current_author)); 
    quote_available = false;  
    initial_quote_loaded = false;
    if (quote_mutex) xSemaphoreGive(quote_mutex);
    BaseType_t res;
    res = xTaskCreate(weather_update_task, "weather_update", 8192, NULL, 2, NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create weather_update_task");
    } else {
        ESP_LOGI(TAG, "Weather update task started");
    }
    
    res = xTaskCreate(quote_update_task, "quote_update", 8192, NULL, 2, NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create quote_update_task");
    } else {
        ESP_LOGI(TAG, "Quote update task started");
    }
    
    ESP_LOGI(TAG, "Step 3: Starting time update loop");
    for (;;) {
        update_time();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    vTaskDelete(NULL);
}


extern "C" void app_main() {
    ESP_LOGI(TAG, "=== Starting Advanced Clock TTS ===");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    weather_mutex = xSemaphoreCreateMutex();
    if (weather_mutex == NULL) {
        ESP_LOGW(TAG, "Failed to create weather mutex - continuing without mutex (not recommended)");
    }

    quote_mutex = xSemaphoreCreateMutex();
    if (quote_mutex == NULL) {
        ESP_LOGW(TAG, "Failed to create quote mutex");
    }

    Board* board = new Board();
    if (!board->init() || !board->begin() ||
        !lvgl_port_init(board->getLCD(), board->getTouch())) {
        ESP_LOGE(TAG, "Display init failed");
        return;
    }
    ui_init();
    display_initialized = true;
    initialize_time();
    location_service_init();
    weather_service_init();
    quote_service_init();
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0, &ui_Screen2_screen_init);
    set_status("WiFi setup required");
    first_boot = false;

    if (!lvgl_port_start()) return;

    g_hx_tts = new HxTTS(HxTTS::BusType::UART);
    if (g_hx_tts) ESP_LOGI(TAG, "TTS initialized");
    else ESP_LOGE(TAG, "Failed to init TTS");

    uart_json_init(ui_set_wifi_credentials);
    xTaskCreate(sequential_tasks, "sequential_tasks", 16384, NULL, 3, NULL);

    ESP_LOGI(TAG, "=== Advanced Clock TTS started ===");
}
#include "weather_service.h"
#include "location_service.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <ctype.h>

static const char *TAG = "WEATHER";

extern bool wifi_connected;
static char http_buffer[8192];

extern "C" {
    void update_weather_status(const char* status, bool is_error);
}

#define WEATHER_BASE_URL "http://api.openweathermap.org/data/2.5/weather?"
#define FORECAST_BASE_URL "http://api.openweathermap.org/data/2.5/forecast?"

char* url_encode(const char* input) {
    if (!input) return NULL;

    size_t len = strlen(input);
    char* encoded = (char*)malloc(len * 3 + 1);
    if (!encoded) return NULL;

    const char* src = input;
    char* dst = encoded;

    while (*src) {
        unsigned char c = (unsigned char)*src;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            *dst++ = c;
        } else if (c == ' ') {
            *dst++ = '%'; *dst++ = '2'; *dst++ = '0';
        } else {
            sprintf(dst, "%%%02X", c);
            dst += 3;
        }
        src++;
    }
    *dst = '\0';
    return encoded;
}

bool is_weather_api_configured(void) {
    size_t len = strlen(OPENWEATHER_API_KEY);
    if (len != 32) {
        ESP_LOGE(TAG, "API key invalid, length: %d", (int)len);
        return false;
    }
    return true;
}

bool http_get_json(const char* url, char* buffer, size_t buffer_size) {
    if (!wifi_connected) {
        update_weather_status("No WiFi", true);
        return false;
    }

    char host[64] = {0};
    char path[256] = {0};

    if (sscanf(url, "http://%63[^/]/%255[^\n]", host, path) < 1) {
        update_weather_status("URL parse failed", true);
        return false;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        update_weather_status("Socket create failed", true);
        return false;
    }

    struct hostent *he = gethostbyname(host);
    if (he == NULL) {
        update_weather_status("DNS lookup failed", true);
        close(sockfd);
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr, he->h_addr, he->h_length);

    struct timeval timeout = {.tv_sec = 15, .tv_usec = 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        update_weather_status("Connection failed", true);
        close(sockfd);
        return false;
    }

    char request[512];
    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: ESP32-Weather\r\n"
             "Connection: close\r\n"
             "Cache-Control: no-cache\r\n"
             "\r\n",
             path, host);

    if (send(sockfd, request, strlen(request), 0) < 0) {
        update_weather_status("Send failed", true);
        close(sockfd);
        return false;
    }

    memset(buffer, 0, buffer_size);
    int total_read = 0;
    int bytes_read;
    bool found_json = false;
    int http_status = 0;

    while ((bytes_read = recv(sockfd, buffer + total_read, buffer_size - total_read - 1, 0)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size - 1) break;
        
        if (http_status == 0 && total_read > 12) {
            sscanf(buffer, "HTTP/1.1 %d", &http_status);
        }

        if (!found_json) {
            char *json_start = strstr(buffer, "\r\n\r\n");
            if (json_start) {
                found_json = true;
                json_start += 4;
                int json_len = total_read - (json_start - buffer);
                memmove(buffer, json_start, json_len);
                total_read = json_len;
                buffer[total_read] = '\0';
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    close(sockfd);

    if (http_status != 200) {
        char msg[32];
        snprintf(msg, sizeof(msg), "HTTP %d", http_status); 
        update_weather_status(msg, true);
        return false;
    }

    if (!found_json || total_read == 0) {
        update_weather_status("No JSON in response", true);
        return false;
    }

    return true;
}

const char* normalize_city_name(const char* raw_city) {
    if (!raw_city) return raw_city;
    
    static char normalized[64];
    
    if (strncmp(raw_city, "St ", 3) == 0) {
        snprintf(normalized, sizeof(normalized), "Saint %s", raw_city + 3);
        return normalized;
    }
    else if (strncmp(raw_city, "St. ", 4) == 0) {
        snprintf(normalized, sizeof(normalized), "Saint %s", raw_city + 4);
        return normalized;
    }
    
    return raw_city;
}

bool parse_weather_response(const char* json_str, weather_data_t* weather) {
    if (!json_str || !weather) {
        update_weather_status("Invalid parse params", true);
        return false;
    }

    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        update_weather_status("JSON parse failed", true);
        return false;
    }

    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (name && cJSON_IsString(name))
        strlcpy(weather->city, name->valuestring, sizeof(weather->city));

    cJSON *main = cJSON_GetObjectItem(root, "main");
    if (main) {
        cJSON *temp = cJSON_GetObjectItem(main, "temp");
        cJSON *feels = cJSON_GetObjectItem(main, "feels_like");
        cJSON *hum = cJSON_GetObjectItem(main, "humidity");
        cJSON *press = cJSON_GetObjectItem(main, "pressure");
        cJSON *temp_min = cJSON_GetObjectItem(main, "temp_min");
        cJSON *temp_max = cJSON_GetObjectItem(main, "temp_max");

        if (temp) weather->temp_current = temp->valuedouble;
        if (feels) weather->temp_feels_like = feels->valuedouble;
        if (hum) weather->humidity = hum->valueint;
        if (press) weather->pressure = press->valueint;
        if (temp_min) weather->temp_min = temp_min->valuedouble;
        if (temp_max) weather->temp_max = temp_max->valuedouble;
    }

    cJSON *arr = cJSON_GetObjectItem(root, "weather");
    if (arr && cJSON_IsArray(arr)) {
        cJSON *w = cJSON_GetArrayItem(arr, 0);
        if (w) {
            cJSON *desc = cJSON_GetObjectItem(w, "description");
            cJSON *code = cJSON_GetObjectItem(w, "id");
            if (desc && cJSON_IsString(desc))
                strlcpy(weather->description, desc->valuestring, sizeof(weather->description));
            if (code && cJSON_IsNumber(code))
                weather->weather_code = code->valueint;
        }
    }

    cJSON_Delete(root);
    return true;
}


bool parse_forecast_response(const char* json_str, weather_forecast_t* forecast) {

    if (!json_str || !forecast) {
        update_weather_status("Parse: NULL params", true);
        return false;
    }

    memset(forecast, 0, sizeof(weather_forecast_t));

    if (strstr(json_str, "\"list\"") == NULL) {
        update_weather_status("Parse: No 'list' in JSON", true);
        return false;
    }

    bool found_9am = false, found_3pm = false, found_9pm = false;
    char found_dates[3][11] = {0};
    int date_count = 0;
    const char* ptr = json_str;
    int items_found = 0;
    
    while ((ptr = strstr(ptr, "\"dt_txt\"")) != NULL && items_found < 40) {
        ptr += 8; 
        
        ptr = strstr(ptr, ":");
        if (!ptr) break;
        ptr++;
        
        while (*ptr == ' ' || *ptr == '\"') ptr++;
        
        char datetime[32];
        int i = 0;
        while (*ptr != '\"' && *ptr != '\0' && i < 31) {
            datetime[i++] = *ptr++;
        }
        datetime[i] = '\0';
        
        int hours = 0;
        char current_date[11];
        if (strlen(datetime) >= 16) {
            sscanf(datetime + 11, "%d:", &hours);
            strncpy(current_date, datetime, 10);
            current_date[10] = '\0';
        } else {
            continue;
        }
        const char* temp_ptr = strstr(ptr, "\"temp\"");
        if (temp_ptr) {
            float temp = 0;
            if (sscanf(temp_ptr, "\"temp\":%f", &temp) == 1) {
                const char* desc_ptr = strstr(temp_ptr, "\"description\"");
                char description[32] = "unknown";
                if (desc_ptr) {
                    const char* desc_start = strstr(desc_ptr, ":");
                    if (desc_start) {
                        desc_start++;
                        while (*desc_start == ' ' || *desc_start == '\"') desc_start++;
                        
                        int j = 0;
                        while (*desc_start != '\"' && *desc_start != '\0' && j < 31) {
                            description[j++] = *desc_start++;
                        }
                        description[j] = '\0';
                    }
                }
                if (hours == 9 && !found_9am) {
                    forecast_slot_t* slot = &forecast->today_slots[0];
                    snprintf(slot->time, sizeof(slot->time), "9:00");
                    slot->temp = temp;
                    strlcpy(slot->description, description, sizeof(slot->description));
                    found_9am = true;
                    forecast->today_slots_count++;

                }
                else if (hours == 15 && !found_3pm) {
                    forecast_slot_t* slot = &forecast->today_slots[1];
                    snprintf(slot->time, sizeof(slot->time), "15:00");
                    slot->temp = temp;
                    strlcpy(slot->description, description, sizeof(slot->description));
                    found_3pm = true;
                    forecast->today_slots_count++;
                    
                }
                else if (hours == 21 && !found_9pm) {
                    forecast_slot_t* slot = &forecast->today_slots[2];
                    snprintf(slot->time, sizeof(slot->time), "21:00");
                    slot->temp = temp;
                    strlcpy(slot->description, description, sizeof(slot->description));
                    found_9pm = true;
                    forecast->today_slots_count++;
                    
                }
                
                if (forecast->daily_count < 3) {
                    bool is_new_date = true;
                    for (int j = 0; j < date_count; j++) {
                        if (strcmp(found_dates[j], current_date) == 0) {
                            is_new_date = false;
                            break;
                        }
                    }
                    
                    if (is_new_date && (hours == 12 || forecast->daily_count == date_count)) {
                        strcpy(found_dates[date_count], current_date);
                        date_count++;
                        
                        daily_forecast_t* day = &forecast->daily[forecast->daily_count];
                        strlcpy(day->date, current_date, sizeof(day->date));
                        day->temp_day = temp;
                        
                        float min_temp = temp;
                        float max_temp = temp;
                        
                        const char* search_ptr = temp_ptr;
                        for (int k = 0; k < 8; k++) { 
                            search_ptr = strstr(search_ptr + 1, "\"dt_txt\"");
                            if (!search_ptr) break;
                            
                            char next_datetime[32];
                            const char* dt_start = strstr(search_ptr, "\"");
                            if (dt_start) {
                                dt_start++;
                                int n = 0;
                                while (*dt_start != '\"' && n < 31) {
                                    next_datetime[n++] = *dt_start++;
                                }
                                next_datetime[n] = '\0';
                                
                                if (strncmp(next_datetime, current_date, 10) == 0) {
                                    const char* next_temp_ptr = strstr(search_ptr, "\"temp\"");
                                    if (next_temp_ptr) {
                                        float next_temp = 0;
                                        if (sscanf(next_temp_ptr, "\"temp\":%f", &next_temp) == 1) {
                                            if (next_temp < min_temp) min_temp = next_temp;
                                            if (next_temp > max_temp) max_temp = next_temp;
                                        }
                                    }
                                } else {
                                    break;
                                }
                            }
                        }
                        
                        day->temp_min = min_temp;
                        day->temp_max = max_temp;
                        strlcpy(day->description, description, sizeof(day->description));
                        forecast->daily_count++;
                    }
                }
                
                items_found++;
            }
        }
        
        if (*ptr == '\0') break;
    }

    char result_msg[128];
    snprintf(result_msg, sizeof(result_msg), "Parse: %d/3 slots, %d/3 days", 
             forecast->today_slots_count, forecast->daily_count);
    update_weather_status(result_msg, false);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    return (forecast->today_slots_count > 0 || forecast->daily_count > 0);
}

bool get_current_weather(const char* city, weather_data_t* weather) {
    if (!city || !weather) return false;

    if (!is_weather_api_configured()) {
        update_weather_status("API not configured", true);
        return false;
    }
    char *encoded = url_encode(city);
    if (!encoded) {
        update_weather_status("URL encode failed", true);
        return false;
    }
    char url[512];
    snprintf(url, sizeof(url),
             "%sq=%s&appid=%s&units=metric&lang=en",
             WEATHER_BASE_URL, encoded, OPENWEATHER_API_KEY);
    
    ESP_LOGI("WEATHER", "Current weather API URL: %s", url);
    free(encoded);

    if (!http_get_json(url, http_buffer, sizeof(http_buffer))) {
        update_weather_status("HTTP failed", true);
        return false;
    }
    memset(weather, 0, sizeof(*weather));
    if (!parse_weather_response(http_buffer, weather)) {
        return false;
    }

    return true;
}

bool get_weather_by_location(weather_data_t* weather) {
    if (!weather) {
        update_weather_status("NULL weather", true);
        return false;
    }
    const char* detected_city = get_city_name();
    const char* city_to_use = normalize_city_name(detected_city);
    
    if (!city_to_use || strcmp(city_to_use, "Unknown") == 0) {
        update_weather_status("No city", true);
        return false;
    }
    memset(weather, 0, sizeof(weather_data_t));
    update_weather_status("Fetching current...", false);
    bool result = get_current_weather(city_to_use, weather);
    
    if (!result) {
        update_weather_status("Current failed", true);
    }

    return result;
}


bool get_weather_forecast(const char* city, weather_forecast_t* forecast) {
    ESP_LOGI("FORECAST_API", "=== START FORECAST API CALL ===");

    if (!city || !forecast) {
        update_weather_status("Forecast: NULL params", true);
        return false;
    }

    if (!is_weather_api_configured()) {
        update_weather_status("Forecast: API key error", true);
        return false;
    }

    char status_msg[128];
    snprintf(status_msg, sizeof(status_msg), "Forecast: City %s", city);


    char *encoded = url_encode(city);
    if (!encoded) {
        update_weather_status("Forecast: URL encode failed", true);
        return false;
    }

    // Формируем URL
    char url[512];
    snprintf(url, sizeof(url),
             "%sq=%s&appid=%s&units=metric&lang=en&cnt=40",
             FORECAST_BASE_URL, encoded, OPENWEATHER_API_KEY);
    

    bool http_result = http_get_json(url, http_buffer, sizeof(http_buffer));
    free(encoded);

    if (!http_result) {
        update_weather_status("Forecast: HTTP request failed", true);
        return false;
    }

    // Проверяем длину ответа
    int response_len = strlen(http_buffer);
    snprintf(status_msg, sizeof(status_msg), "Forecast: Got %d bytes", response_len);

    if (response_len < 50) {
        snprintf(status_msg, sizeof(status_msg), "Forecast: Short response: %s", http_buffer);
        update_weather_status(status_msg, true);
        return false;
    }
    
    memset(forecast, 0, sizeof(*forecast));
    
    if (!parse_forecast_response(http_buffer, forecast)) {
        update_weather_status("Forecast: Parse failed", true);
        return false;
    }

    // Показываем результат
    snprintf(status_msg, sizeof(status_msg), "Forecast: %d slots, %d days", 
             forecast->today_slots_count, forecast->daily_count);
    update_weather_status(status_msg, false);
    
    return true;
}

bool get_forecast_by_location(weather_forecast_t* forecast) {
    if (!forecast) {
        update_weather_status("Forecast: NULL forecast", true);
        return false;
    }

    const char* detected_city = get_city_name();
    const char* city_to_use = normalize_city_name(detected_city);
    if (!city_to_use) {
        update_weather_status("Forecast: City NULL", true);
        return false;
    }

    if (strcmp(city_to_use, "Unknown") == 0) {
        update_weather_status("Forecast: City Unknown", true);
        return false;
    }
    
    return get_weather_forecast(city_to_use, forecast);
}

void weather_service_init(void) {
    ESP_LOGI(TAG, "Weather service initialized");
    if (!is_weather_api_configured())
        ESP_LOGW(TAG, "API key not configured correctly");
}
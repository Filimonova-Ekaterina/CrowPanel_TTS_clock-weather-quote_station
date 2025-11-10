#include "location_service.h"
#include "esp_log.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <ctype.h>

static const char *TAG = "LOCATION";

static location_data_t current_location = {0};
static bool location_available = false;

const char* get_city_name(void) {
    if (!location_available) {
        location_data_t loc;
        if (get_location_data(&loc)) {
            char debug_msg[128];
            snprintf(debug_msg, sizeof(debug_msg), "City from loc: %s", current_location.city);
            update_weather_status(debug_msg, false);
            return current_location.city;
        }
        return "Unknown";
    }
    char debug_msg[128];
    snprintf(debug_msg, sizeof(debug_msg), "City cached: %s", current_location.city);
    update_weather_status(debug_msg, false);
    return current_location.city;
}

static bool http_get_json(const char* host, const char* path, char* buffer, size_t buffer_size) {

    
    ESP_LOGI(TAG, "HTTP GET: %s%s", host, path);
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return false;
    }

    struct hostent *he = gethostbyname(host);
    if (he == NULL) {
        ESP_LOGE(TAG, "Failed to resolve host: %s", host);
        close(sockfd);
        return false;
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    server_addr.sin_addr.s_addr = *((unsigned long*)he->h_addr_list[0]);

    struct timeval timeout = {.tv_sec = 10, .tv_usec = 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Failed to connect to %s", host);
        close(sockfd);
        return false;
    }

    char request[256];
    snprintf(request, sizeof(request), 
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: ESP32\r\n"
             "Connection: close\r\n"
             "\r\n", path, host);
    
    if (send(sockfd, request, strlen(request), 0) < 0) {
        ESP_LOGE(TAG, "Failed to send request");
        close(sockfd);
        return false;
    }

    int total_read = 0;
    int bytes_read;
    bool found_json = false;

    while ((bytes_read = recv(sockfd, buffer + total_read, buffer_size - total_read - 1, 0)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size - 1) break;
        
        if (!found_json) {
            char* json_start = strstr(buffer, "\r\n\r\n");
            if (json_start) {
                found_json = true;
                json_start += 4;
                int json_len = total_read - (json_start - buffer);
                memmove(buffer, json_start, json_len);
                total_read = json_len;
            }
        }
    }
    
    close(sockfd);

    if (total_read == 0 || !found_json) {
        ESP_LOGE(TAG, "No JSON data received");
        return false;
    }

    buffer[total_read] = '\0';
    return true;
}

static bool parse_ipapi_response(const char* json_str, location_data_t* location) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return false;
    }

    bool success = false;

    cJSON *status = cJSON_GetObjectItem(root, "status");
    if (status && strcmp(status->valuestring, "success") == 0) {
        cJSON *city = cJSON_GetObjectItem(root, "city");
        cJSON *country = cJSON_GetObjectItem(root, "country");
        cJSON *region = cJSON_GetObjectItem(root, "regionName");
        cJSON *lat = cJSON_GetObjectItem(root, "lat");
        cJSON *lon = cJSON_GetObjectItem(root, "lon");
        cJSON *timezone = cJSON_GetObjectItem(root, "timezone");

        if (city && country && lat && lon && timezone) {
            strlcpy(location->city, city->valuestring, sizeof(location->city));
            strlcpy(location->country, country->valuestring, sizeof(location->country));
            if (region) {
                strlcpy(location->region, region->valuestring, sizeof(location->region));
            }
            location->latitude = lat->valuedouble;
            location->longitude = lon->valuedouble;
            strlcpy(location->timezone, timezone->valuestring, sizeof(location->timezone));
            
            success = true;
            ESP_LOGI(TAG, "Location: %s, %s (%s)", location->city, location->country, location->region);
            ESP_LOGI(TAG, "Coordinates: %.4f, %.4f", location->latitude, location->longitude);
            ESP_LOGI(TAG, "Timezone: %s", location->timezone);
        }
    } else {
        cJSON *message = cJSON_GetObjectItem(root, "message");
        ESP_LOGE(TAG, "IP-API error: %s", message ? message->valuestring : "Unknown error");
    }

    cJSON_Delete(root);
    return success;
}

bool get_location_data(location_data_t* location) {
    if (!location) {
        update_weather_status("Location: NULL pointer", true);
        return false;
    }
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        char manual_city[64];
        size_t size = sizeof(manual_city);
        if (nvs_get_str(handle, "manual_city", manual_city, &size) == ESP_OK) {
            nvs_close(handle);
            strlcpy(location->city, manual_city, sizeof(location->city));
            location->country[0] = '\0';
            location->region[0] = '\0';
            location->latitude = 0;
            location->longitude = 0;
            strlcpy(location->timezone, "Manual", sizeof(location->timezone));
            update_weather_status("Manual location OK", false);
            ESP_LOGI(TAG, "Using manual location: %s", manual_city);
            return true;
        }
        nvs_close(handle);
    }

    extern bool wifi_connected;
    if (!wifi_connected) {
        update_weather_status("Location: No WiFi", true);
        return false;
    }

    char buffer[2048];
    if (http_get_json("ip-api.com", "/json", buffer, sizeof(buffer))) {
        if (parse_ipapi_response(buffer, location)) {
            memcpy(&current_location, location, sizeof(location_data_t));
            location_available = true;
            return true;
        }
    }

    return false;
}


const char* get_timezone_by_location(void) {
    if (!location_available) {
        location_data_t loc;
        if (get_location_data(&loc)) {
            return current_location.timezone;
        }
        return "GMT0";
    }
    return current_location.timezone;
}

void location_service_init(void) {
    ESP_LOGI(TAG, "Location service initialized");
}

bool set_manual_location(const char* city) {
    if (!city || strlen(city) == 0) {
        return clear_manual_location();
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }
    char formatted_city[64];
    strlcpy(formatted_city, city, sizeof(formatted_city));
    if (strlen(formatted_city) > 0) {
        formatted_city[0] = toupper(formatted_city[0]);
        for (int i = 1; formatted_city[i] != '\0'; ++i) {
            formatted_city[i] = tolower(formatted_city[i]);
        }
    }

    err = nvs_set_str(handle, "manual_city", formatted_city);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set manual_city: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit manual_city: %s", esp_err_to_name(err));
        return false;
    }
    location_available = false;
    
    ESP_LOGI(TAG, "Manual location saved: %s", formatted_city);
    return true;
}

bool clear_manual_location(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }

    err = nvs_erase_key(handle, "manual_city");
    if (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_commit(handle);
        nvs_close(handle);
        location_available = false;
        ESP_LOGI(TAG, "Manual location cleared");
        return true;
    }

    nvs_close(handle);
    ESP_LOGE(TAG, "Failed to clear manual location: %s", esp_err_to_name(err));
    return false;
}
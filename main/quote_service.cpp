#include "quote_service.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <ctype.h>
#include <stddef.h>

static const char *TAG = "QUOTE_SERVICE";

extern bool wifi_connected;
static char http_buffer[2048];

extern "C" {
    void update_quote_status(const char* status, bool is_error);
    void update_quote_text(const char* quote_text);
}

static bool get_fallback_quote_simple(char* quote_buffer, size_t buffer_size) {
    ESP_LOGI(TAG, "get_fallback_quote_simple called");
    
    const char* fallback_quotes[] = {
        "The only way to do great work is to love what you do. - Steve Jobs",
        "Innovation distinguishes between a leader and a follower. - Steve Jobs", 
        "Your time is limited, so don't waste it living someone else's life. - Steve Jobs",
        "Stay hungry, stay foolish. - Steve Jobs",
        "The future belongs to those who believe in the beauty of their dreams. - Eleanor Roosevelt",
        "Life is what happens to you while you're busy making other plans. - John Lennon",
        "The only impossible journey is the one you never begin. - Tony Robbins",
        "It does not matter how slowly you go as long as you do not stop. - Confucius",
        "Everything you've ever wanted is on the other side of fear. - George Addair",
        "The way to get started is to quit talking and begin doing. - Walt Disney"
    };
    
    int quote_count = sizeof(fallback_quotes) / sizeof(fallback_quotes[0]);
    int random_index = esp_random() % quote_count;
    
    strlcpy(quote_buffer, fallback_quotes[random_index], buffer_size);
    ESP_LOGI(TAG, "Using fallback quote #%d", random_index + 1);
    
    return true;
}

bool http_get_json_quote(const char* url, char* buffer, size_t buffer_size) {
    if (!wifi_connected) {
        update_quote_status("No WiFi connection", true);
        return false;
    }

    char host[64] = {0};
    char path[256] = {0};

    if (sscanf(url, "http://%63[^/]/%255[^\n]", host, path) < 1) {
        ESP_LOGE(TAG, "URL parse failed: %s", url);
        update_quote_status("URL parse error", true);
        return false;
    }

    ESP_LOGI(TAG, "Connecting to: %s, path: /%s", host, path);
    update_quote_status("Connecting to API...", false);

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Socket create failed");
        update_quote_status("Socket error", true);
        return false;
    }

    struct hostent *he = gethostbyname(host);
    if (he == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed: %s", host);
        update_quote_status("DNS failed", true);
        close(sockfd);
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr, he->h_addr, he->h_length);

    struct timeval timeout = {.tv_sec = 10, .tv_usec = 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Connection failed: %s", host);
        update_quote_status("Connection failed", true);
        close(sockfd);
        return false;
    }

    ESP_LOGI(TAG, "Connected, sending request...");
    update_quote_status("Requesting quote...", false);

    char request[512];
    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: ESP32-Quote-Service/1.0\r\n"
             "Accept: application/json\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);

    int sent = send(sockfd, request, strlen(request), 0);
    if (sent < 0) {
        ESP_LOGE(TAG, "Send failed: %d", errno);
        update_quote_status("Send failed", true);
        close(sockfd);
        return false;
    }

    ESP_LOGI(TAG, "Sent %d bytes, reading...", sent);

    memset(buffer, 0, buffer_size);
    int total_read = 0;
    int bytes_read;
    bool found_json = false;
    int http_status = 0;

    while ((bytes_read = recv(sockfd, buffer + total_read, buffer_size - total_read - 1, 0)) > 0) {
        total_read += bytes_read;
        if (total_read >= buffer_size - 1) break;
        
        if (http_status == 0 && total_read > 12) {
            if (sscanf(buffer, "HTTP/1.1 %d", &http_status) == 1) {
                ESP_LOGI(TAG, "HTTP Status: %d", http_status);
                char status_msg[32];
                snprintf(status_msg, sizeof(status_msg), "HTTP %d", http_status);
                update_quote_status(status_msg, http_status != 200);
            }
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
                ESP_LOGI(TAG, "JSON found: %d bytes", json_len);
                update_quote_status("Parsing quote...", false);
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    close(sockfd);

    if (bytes_read < 0) {
        ESP_LOGE(TAG, "Receive error: %d", errno);
        update_quote_status("Receive failed", true);
        return false;
    }

    ESP_LOGI(TAG, "Total received: %d bytes", total_read);

    if (http_status != 200) {
        char msg[32];
        snprintf(msg, sizeof(msg), "HTTP Error %d", http_status);
        ESP_LOGE(TAG, "HTTP error: %d", http_status);
        update_quote_status(msg, true);
        return false;
    }

    if (!found_json || total_read == 0) {
        ESP_LOGE(TAG, "No JSON in response");
        update_quote_status("No JSON data", true);
        return false;
    }

    ESP_LOGI(TAG, "Quote received successfully");
    update_quote_status("Quote received", false);
    return true;
}

static bool parse_quote_response(const char* json_str, quote_data_t* quote) {
    if (!json_str || strlen(json_str) == 0 || !quote) {
        ESP_LOGE(TAG, "Invalid parse parameters");
        return false;
    }

    ESP_LOGD(TAG, "Raw JSON: %s", json_str);

    cJSON* root = cJSON_Parse(json_str);
    if (root == NULL) {
        ESP_LOGE(TAG, "JSON parse failed");
        update_quote_status("JSON parse error", true);
        return false;
    }

    bool success = false;
    
    cJSON* content = cJSON_GetObjectItem(root, "content");
    cJSON* author = cJSON_GetObjectItem(root, "author");
    cJSON* tags = cJSON_GetObjectItem(root, "tags");
    cJSON* length = cJSON_GetObjectItem(root, "length");

    if (content && cJSON_IsString(content) && author && cJSON_IsString(author)) {
        const char* quote_content = content->valuestring;
        const char* quote_author = author->valuestring;
        
        if (strlen(quote_content) > 0 && strlen(quote_author) > 0) {
            clean_quote_text(quote_content, quote->content, sizeof(quote->content));
            strlcpy(quote->author, quote_author, sizeof(quote->author));
            
            if (tags && cJSON_IsArray(tags)) {
                char tags_str[128] = "";
                cJSON* tag = NULL;
                int tag_count = 0;
                
                cJSON_ArrayForEach(tag, tags) {
                    if (cJSON_IsString(tag) && tag_count < 3) {
                        if (tag_count > 0) {
                            strlcat(tags_str, ", ", sizeof(tags_str));
                        }
                        strlcat(tags_str, tag->valuestring, sizeof(tags_str));
                        tag_count++;
                    }
                }
                strlcpy(quote->tags, tags_str, sizeof(quote->tags));
            }
            
            if (length && cJSON_IsNumber(length)) {
                quote->length = length->valueint;
            }
            
            quote->is_fallback = false;
            success = true;
            
            ESP_LOGI(TAG, "Quote extracted: %.50s... by %s", 
                     quote->content, quote->author);
        }
    } else {
        ESP_LOGE(TAG, "Missing content or author in JSON");
        update_quote_status("Missing quote data", true);
    }

    cJSON_Delete(root);
    return success;
}

void clean_quote_text(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        if (output && output_size > 0) output[0] = '\0';
        return;
    }

    char* dest = output;
    const char* src = input;
    size_t remaining = output_size - 1;
    
    bool last_was_space = false;
    
    while (*src != '\0' && remaining > 1) {
        if (*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r') {
            if (!last_was_space) {
                *dest++ = ' ';
                remaining--;
                last_was_space = true;
            }
        } else {
            *dest++ = *src;
            remaining--;
            last_was_space = false;
        }
        src++;
    }
    
    if (dest > output && *(dest-1) == ' ') {
        dest--;
    }
    
    *dest = '\0';
}

bool get_random_quote(quote_data_t* quote) {
    if (!is_quote_api_configured()) {
        ESP_LOGE(TAG, "API not configured");
        update_quote_status("API not configured", true);
        return false;
    }

    if (quote == NULL) {
        ESP_LOGE(TAG, "Invalid quote pointer");
        update_quote_status("Invalid pointer", true);
        return false;
    }

    memset(quote, 0, sizeof(quote_data_t));

    ESP_LOGI(TAG, "Fetching random quote...");
    update_quote_status("Starting quote fetch...", false);

    char url[256];
    snprintf(url, sizeof(url), "%s", QUOTE_API_URL);
    
    ESP_LOGI(TAG, "API URL: %s", url);

    bool http_success = http_get_json_quote(url, http_buffer, sizeof(http_buffer));

    if (!http_success) {
        ESP_LOGE(TAG, "HTTP request failed");
        update_quote_status("HTTP request failed", true);
        return false;
    }

    if (!parse_quote_response(http_buffer, quote)) {
        ESP_LOGE(TAG, "JSON parse failed");
        update_quote_status("JSON parse failed", true);
        return false;
    }

    ESP_LOGI(TAG, "Quote loaded successfully from API");
    update_quote_status("Quote loaded OK", false);
    return true;
}

bool get_random_quote_simple(char* quote_buffer, size_t buffer_size) {
    ESP_LOGI(TAG, "get_random_quote_simple called, buffer_size=%d", buffer_size);
    
    quote_data_t quote;
    memset(&quote, 0, sizeof(quote_data_t));
    
    if (get_random_quote(&quote)) {
        ESP_LOGI(TAG, "API success: %.100s", quote.content);
        snprintf(quote_buffer, buffer_size, "\"%s\" - %s", quote.content, quote.author);
        return true;
    }
    
    ESP_LOGW(TAG, "API failed, using fallback");
    return get_fallback_quote_simple(quote_buffer, buffer_size);
}

bool is_quote_api_configured(void) {
    return (strlen(QUOTE_API_URL) > 0);
}

void quote_service_init(void) {
    ESP_LOGI(TAG, "Quote service initialized");
    ESP_LOGI(TAG, "API URL: %s", QUOTE_API_URL);
    
    if (!is_quote_api_configured()) {
        ESP_LOGW(TAG, "Quote API URL not configured correctly");
    }
}

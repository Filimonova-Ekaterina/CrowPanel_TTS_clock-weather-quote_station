#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "cJSON.h"
#include "driver/uart.h"
#include "esp_crc.h"
#include "esp_log.h"

#include "string.h"
#include "stdlib.h"

#include "uart_json.h"
#include "uart_manager.h"
#include "ui_events.h"

#define UART_JSON_BUF 2048

#ifndef UART_JSON_NUM
#define UART_JSON_NUM UART_NUM_0
#endif

#ifndef UART_JSON_BAUD
#define UART_JSON_BAUD 115200
#endif

#ifndef UART_JSON_RX_PIN
#define UART_JSON_RX_PIN UART_PIN_NO_CHANGE
#endif
#ifndef UART_JSON_TX_PIN
#define UART_JSON_TX_PIN UART_PIN_NO_CHANGE
#endif

static const char *TAG = "UART_JSON_MODULE";
static QueueHandle_t uart_json_queue = NULL;
static uart_json_on_wifi_cb_t wifi_callback = NULL;

static void uart_json_send_str(const char *s) {
    if (!s) return;
    size_t len = strlen(s);
    uart_manager_write(UART_JSON_NUM, s, len);
    ESP_LOGI(TAG, "TX -> %s", s);
}

static void trim_newline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

static bool validate_and_parse_wifi_config(const char *json_in) {
    if (!json_in) return false;

    char *copy = strdup(json_in);
    if (!copy) return false;
    trim_newline(copy);

    ESP_LOGI(TAG, "Raw JSON received: %s", copy);

    cJSON *root = cJSON_Parse(copy);
    if (!root) {
        ESP_LOGE(TAG, "cJSON_Parse failed");
        free(copy);
        return false;
    }
    cJSON *type_item = cJSON_GetObjectItem(root, "type");
    cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
    cJSON *password_item = cJSON_GetObjectItem(root, "password");
    cJSON *crc_item = cJSON_GetObjectItem(root, "crc32");

    if (!cJSON_IsString(type_item) || 
        !cJSON_IsString(ssid_item) || 
        !cJSON_IsString(password_item) || 
        !cJSON_IsString(crc_item)) {
        ESP_LOGE(TAG, "Missing or invalid required fields");
        cJSON_Delete(root);
        free(copy);
        return false;
    }

    if (strcmp(type_item->valuestring, "wifi_config") != 0) {
        ESP_LOGE(TAG, "Invalid message type: %s", type_item->valuestring);
        cJSON_Delete(root);
        free(copy);
        return false;
    }

    char *crc_str = strdup(crc_item->valuestring);
    if (!crc_str) {
        ESP_LOGE(TAG, "No memory for crc_str");
        cJSON_Delete(root);
        free(copy);
        return false;
    }

    cJSON_DeleteItemFromObject(root, "crc32");

    char *pure = cJSON_PrintUnformatted(root);
    if (!pure) {
        ESP_LOGE(TAG, "cJSON_PrintUnformatted failed");
        free(crc_str);
        cJSON_Delete(root);
        free(copy);
        return false;
    }

    uint32_t calc_crc = esp_crc32_le(0, (uint8_t *)pure, strlen(pure));
    char calc_str[9];
    snprintf(calc_str, sizeof(calc_str), "%08lX", (unsigned long)calc_crc);
    ESP_LOGI(TAG, "Pure JSON for CRC: %s", pure);
    ESP_LOGI(TAG, "Calc CRC32: %s, Received CRC32: %s", calc_str, crc_str);

    if (strcasecmp(calc_str, crc_str) != 0) {
        ESP_LOGE(TAG, "CRC mismatch: received=%s expected=%s", crc_str, calc_str);
        uart_json_send_str("CRC ERROR\n");
        free(pure);
        free(crc_str);
        cJSON_Delete(root);
        free(copy);
        return false;
    }

    if (wifi_callback) {
        wifi_callback(ssid_item->valuestring, password_item->valuestring);
        ESP_LOGI(TAG, "WiFi config received: SSID=%s", ssid_item->valuestring);
    }

    free(pure);
    free(crc_str);
    cJSON_Delete(root);
    free(copy);
    return true;
}

static void uart_json_task(void *arg) {
    uint8_t *buf = malloc(UART_JSON_BUF + 1);
    if (!buf) {
        ESP_LOGE(TAG, "No memory for buffer");
        vTaskDelete(NULL);
        return;
    }
    size_t buffered = 0;

    ESP_LOGI(TAG, "UART JSON task started (UART %d, baud %d)", UART_JSON_NUM, UART_JSON_BAUD);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    while (1) {
        uart_event_t evt;
        if (xQueueReceive(uart_json_queue, &evt, portMAX_DELAY) != pdTRUE) continue;

        switch (evt.type) {
            case UART_DATA: {
                int r = uart_read_bytes(UART_JSON_NUM, &buf[buffered], evt.size, pdMS_TO_TICKS(1000));
                if (r > 0) buffered += r;
                if (buffered >= UART_JSON_BUF) {
                    ESP_LOGW(TAG, "Buffer overflow, clearing");
                    buffered = 0;
                    memset(buf, 0, UART_JSON_BUF);
                    uart_json_send_str("UART overflow\n");
                    break;
                }
                if (evt.timeout_flag) {
                    buf[buffered] = '\0';
                    ESP_LOGI(TAG, "[UART DATA] size=%u", (unsigned)buffered);
                    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, buffered, ESP_LOG_INFO);
                    if (validate_and_parse_wifi_config((char *)buf)) {
                        uart_json_send_str("WiFi config accepted\n");
                        ESP_LOGI(TAG, "WiFi config accepted");
                    } else {
                        uart_json_send_str("Error parsing WiFi config\n");
                    }
                    buffered = 0;
                    memset(buf, 0, UART_JSON_BUF);
                }
                break;
            }
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "UART buffer full");
                uart_flush_input(UART_JSON_NUM);
                uart_json_send_str("UART overflow\n");
                break;
            default:
                ESP_LOGI(TAG, "UART event type: %d", evt.type);
                break;
        }
    }

    free(buf);
    vTaskDelete(NULL);
}

int uart_json_init(uart_json_on_wifi_cb_t on_wifi) {
    ESP_LOGI(TAG, "Init UART JSON for WiFi config (UART %d, baud %d)", UART_JSON_NUM, UART_JSON_BAUD);
    if (uart_is_driver_installed(UART_JSON_NUM)) {
        ESP_LOGW(TAG, "UART driver already installed, reinstalling...");
        uart_driver_delete(UART_JSON_NUM);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    esp_err_t r = uart_manager_install(UART_JSON_NUM, UART_JSON_RX_PIN, UART_JSON_TX_PIN, UART_JSON_BAUD, &uart_json_queue);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "uart_manager_install failed: %d", r);
        return -1;
    }

    wifi_callback = on_wifi;

    BaseType_t ret = xTaskCreate(uart_json_task, "uart_json_task", 6144, NULL, 10, NULL);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Unable to create uart_json_task");
        return -1;
    }

    ESP_LOGI(TAG, "UART JSON ready for WiFi config");
    return 0;
}
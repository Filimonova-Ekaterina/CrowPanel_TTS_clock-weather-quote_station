#include "uart_manager.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "uart_manager";

#define UART_BUF_SIZE 2048
#define UART_QUEUE_SIZE 20

#define UART_MANAGER_MAX_PORTS 3

static QueueHandle_t uart_queues[UART_MANAGER_MAX_PORTS] = {0};
static int uart_installed[UART_MANAGER_MAX_PORTS] = {0};

esp_err_t uart_manager_install(uart_port_t uart_num, int rx_pin, int tx_pin, int baud, QueueHandle_t *out_queue)
{
    if (uart_num < 0 || uart_num >= UART_MANAGER_MAX_PORTS) {
        ESP_LOGE(TAG, "Invalid UART num %d", uart_num);
        return ESP_ERR_INVALID_ARG;
    }

    if (uart_installed[uart_num]) {
        if (out_queue) *out_queue = uart_queues[uart_num];
        return ESP_OK;
    }

    uart_config_t cfg = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    esp_err_t r = uart_param_config(uart_num, &cfg);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed");
        return r;
    }

    r = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed");
        return r;
    }

    QueueHandle_t q = NULL;
    r = uart_driver_install(uart_num, UART_BUF_SIZE, UART_BUF_SIZE, UART_QUEUE_SIZE, &q, 0);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed");
        return r;
    }

    uart_queues[uart_num] = q;
    uart_installed[uart_num] = 1;
    if (out_queue) *out_queue = q;

    ESP_LOGI(TAG, "uart_manager: installed UART%d (baud=%d)", uart_num, baud);
    return ESP_OK;
}

QueueHandle_t uart_manager_get_queue(uart_port_t uart_num)
{
    if (uart_num < 0 || uart_num >= UART_MANAGER_MAX_PORTS) return NULL;
    return uart_queues[uart_num];
}

esp_err_t uart_manager_write(uart_port_t uart_num, const void* data, size_t len)
{
    if (uart_num < 0 || uart_num >= UART_MANAGER_MAX_PORTS) return ESP_ERR_INVALID_ARG;
    if (!uart_installed[uart_num]) return ESP_ERR_INVALID_STATE;
    int written = uart_write_bytes(uart_num, (const char*)data, len);
    return (written == (int)len) ? ESP_OK : ESP_FAIL;
}

esp_err_t uart_manager_uninstall(uart_port_t uart_num)
{
    if (uart_num < 0 || uart_num >= UART_MANAGER_MAX_PORTS) return ESP_ERR_INVALID_ARG;
    if (!uart_installed[uart_num]) return ESP_OK;
    esp_err_t r = uart_driver_delete(uart_num);
    if (r == ESP_OK) {
        uart_installed[uart_num] = 0;
        uart_queues[uart_num] = NULL;
    }
    return r;
}

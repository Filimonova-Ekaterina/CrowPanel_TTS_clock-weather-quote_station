#pragma once

#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t uart_manager_install(uart_port_t uart_num, int rx_pin, int tx_pin, int baud, QueueHandle_t *out_queue);


QueueHandle_t uart_manager_get_queue(uart_port_t uart_num);


esp_err_t uart_manager_write(uart_port_t uart_num, const void* data, size_t len);

esp_err_t uart_manager_uninstall(uart_port_t uart_num);

#ifdef __cplusplus
}
#endif

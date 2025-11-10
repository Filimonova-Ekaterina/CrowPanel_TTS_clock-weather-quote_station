#include "uart_manager.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include "ui_events.h"

static const char *TAG = "uart_rx_task";

#define UART_NUM_TO_USE UART_NUM_1
#define LINEBUF_SIZE 4096
#define READ_CHUNK 256
void uart_rx_task(void *arg)
{
    (void)arg;
    QueueHandle_t q = uart_manager_get_queue(UART_NUM_TO_USE);
    if (!q) {
        ESP_LOGE(TAG, "no uart queue for UART%d", UART_NUM_TO_USE);
        vTaskDelete(NULL);
        return;
    }

    uart_event_t event;
    static char linebuf[LINEBUF_SIZE];
    size_t line_pos = 0;

    for (;;) {
        if (xQueueReceive(q, &event, portMAX_DELAY)) {
            if (event.type == UART_DATA) {
                uint8_t data[READ_CHUNK];
                int len = uart_read_bytes(UART_NUM_TO_USE, data, (event.size < READ_CHUNK) ? event.size : READ_CHUNK, pdMS_TO_TICKS(100));
                if (len > 0) {
                    for (int i = 0; i < len; ++i) {
                        char c = (char)data[i];
                        if (c == '\r') continue;
                        if (c == '\n') {
                            if (line_pos > 0) {
                                linebuf[line_pos] = 0;
                                line_pos = 0;
                            } else {
                            }
                        } else {
                            if (line_pos < LINEBUF_SIZE - 1) {
                                linebuf[line_pos++] = c;
                            } else {
                                linebuf[line_pos] = 0;
                                line_pos = 0;
                            }
                        }
                    }
                }
            } else if (event.type == UART_FIFO_OVF) {
                ESP_LOGW(TAG, "UART FIFO OVF");
                uart_flush_input(UART_NUM_TO_USE);
            } else if (event.type == UART_BUFFER_FULL) {
                ESP_LOGW(TAG, "UART buffer full");
                uart_flush_input(UART_NUM_TO_USE);
            } else if (event.type == UART_BREAK) {
                ESP_LOGW(TAG, "UART_BREAK");
            } else {
            }
        }
    }
}

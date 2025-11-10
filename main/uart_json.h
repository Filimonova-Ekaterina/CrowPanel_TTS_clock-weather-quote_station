#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*uart_json_on_wifi_cb_t)(const char* ssid, const char* password);

int uart_json_init(uart_json_on_wifi_cb_t on_wifi);

#ifdef __cplusplus
}
#endif

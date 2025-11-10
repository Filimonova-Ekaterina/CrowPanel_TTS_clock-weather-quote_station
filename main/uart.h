#ifndef UART_H_
#define UART_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int uart_init();
int uart_release();
int uart_flush_buffers();
int uart_write(void* data, uint32_t bytes, uint32_t timeout);
int uart_read(void* buffer, uint32_t bytes, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif

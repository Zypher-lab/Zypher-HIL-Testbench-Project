
#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#include <stdbool.h>
#include <stddef.h>

int uart_service_init(void);

bool uart_service_write(const char *channel_name,
                        const char *tx);

bool uart_service_read(const char *channel_name,
                       int timeout_ms,
                       char *rx_out,
                       size_t rx_out_size);

bool uart_service_send_expect(const char *channel_name,
                              const char *tx,
                              const char *expected,
                              int timeout_ms,
                              char *rx_out,
                              size_t rx_out_size);

#endif
#ifndef UART_TRANSPORT_H
#define UART_TRANSPORT_H

#include <stddef.h>

int uart_transport_init(void);
void uart_transport_read_line(char *buffer, size_t buffer_size);
void uart_transport_send_line(const char *line);

#endif
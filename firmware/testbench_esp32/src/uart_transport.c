#include "uart_transport.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#define UART_NODE DT_CHOSEN(zephyr_console)

static const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

int uart_transport_init(void)
{
    if (!device_is_ready(uart_dev)) {
        return -1;
    }

    return 0;
}

void uart_transport_send_line(const char *line)
{
    while (*line != '\0') {
        uart_poll_out(uart_dev, *line);
        line++;
    }
}
void uart_transport_read_line(char *buffer, size_t buffer_size)
{
    size_t index = 0;

    while (1) {
        unsigned char c;

        if (uart_poll_in(uart_dev, &c) == 0) {
            if (c == '\r' || c == '\n') {
                if (index == 0) {
                    continue;
                }

                buffer[index] = '\0';
                return;
            }

            if (index < buffer_size - 1) {
                buffer[index] = (char)c;
                index++;
            }
        }

        k_msleep(1);
    }
}
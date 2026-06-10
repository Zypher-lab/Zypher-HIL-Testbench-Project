#include "uart_echo.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <string.h>

#define UART_NODE   DT_NODELABEL(usart2)
#define BUF_SIZE    64

static const struct device *uart_dev;
static char rx_buf[BUF_SIZE];
static int  rx_idx;

static void uart_cb(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);

    if (!uart_irq_update(dev)) return;

    while (uart_irq_rx_ready(dev)) {
        uint8_t c;
        uart_fifo_read(dev, &c, 1);

        if (c == '\n' || c == '\r') {
            if (rx_idx > 0) {
                rx_buf[rx_idx] = '\0';
                /* echo back */
                for (int i = 0; i < rx_idx; i++) {
                    uart_poll_out(dev, rx_buf[i]);
                }
                uart_poll_out(dev, '\r');
                uart_poll_out(dev, '\n');
                rx_idx = 0;
            }
        } else if (rx_idx < BUF_SIZE - 1) {
            rx_buf[rx_idx++] = c;
        }
    }
}

int uart_echo_init(void)
{
    uart_dev = DEVICE_DT_GET(UART_NODE);
    if (!device_is_ready(uart_dev)) return -1;

    uart_irq_callback_set(uart_dev, uart_cb);
    uart_irq_rx_enable(uart_dev);

    return 0;
}

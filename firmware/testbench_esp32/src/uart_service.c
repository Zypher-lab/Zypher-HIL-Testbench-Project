
#include "uart_service.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <string.h>

#define UART_CH1_NODE DT_NODELABEL(uart2)

static const struct device *uart_ch1_dev = DEVICE_DT_GET(UART_CH1_NODE);

static const struct device *get_uart_device(const char *channel_name)
{
    if (strcmp(channel_name, "UART_CH1") == 0) {
        return uart_ch1_dev;
    }

    return NULL;
}

static void uart_flush_rx(const struct device *uart_dev)
{
    unsigned char c;

    while (uart_poll_in(uart_dev, &c) == 0) {
        /* discard */
    }
}

static void uart_send_line(const struct device *uart_dev, const char *msg)
{
    while (*msg != '\0') {
        uart_poll_out(uart_dev, *msg);
        msg++;
    }

    uart_poll_out(uart_dev, '\n');
}

static bool uart_read_line(const struct device *uart_dev,
                           char *rx_out,
                           size_t rx_out_size,
                           int timeout_ms)
{
    size_t index = 0;
    int64_t deadline = k_uptime_get() + timeout_ms;

    if (rx_out_size == 0) {
        return false;
    }

    while (k_uptime_get() < deadline) {
        unsigned char c;

        if (uart_poll_in(uart_dev, &c) == 0) {
            if (c == '\r' || c == '\n') {
                if (index == 0) {
                    continue;
                }

                rx_out[index] = '\0';
                return true;
            }

            if (index < rx_out_size - 1) {
                rx_out[index++] = (char)c;
            }
        }

        k_msleep(1);
    }

    rx_out[index] = '\0';
    return false;
}

int uart_service_init(void)
{
    if (!device_is_ready(uart_ch1_dev)) {
        return -1;
    }

    uart_flush_rx(uart_ch1_dev);
    return 0;
}

bool uart_service_write(const char *channel_name,
                        const char *tx)
{
    const struct device *uart_dev = get_uart_device(channel_name);

    if (uart_dev == NULL) {
        return false;
    }

    uart_flush_rx(uart_dev);
    uart_send_line(uart_dev, tx);

    return true;
}

bool uart_service_read(const char *channel_name,
                       int timeout_ms,
                       char *rx_out,
                       size_t rx_out_size)
{
    const struct device *uart_dev = get_uart_device(channel_name);

    if (uart_dev == NULL) {
        return false;
    }

    return uart_read_line(uart_dev, rx_out, rx_out_size, timeout_ms);
}

bool uart_service_send_expect(const char *channel_name,
                              const char *tx,
                              const char *expected,
                              int timeout_ms,
                              char *rx_out,
                              size_t rx_out_size)
{
    if (!uart_service_write(channel_name, tx)) {
        return false;
    }

    if (!uart_service_read(channel_name, timeout_ms, rx_out, rx_out_size)) {
        return false;
    }

    return strcmp(rx_out, expected) == 0;
}
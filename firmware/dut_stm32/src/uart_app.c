#include "uart_app.h"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <stdbool.h>
#include <string.h>

/*
 * STM32 UART DUT behavior
 *
 * USART2:
 * - PA2 = USART2_TX
 * - PA3 = USART2_RX
 *
 * Behavior:
 * - Receive "PING"  -> Reply "PONG"
 * - Receive "HELLO" -> Reply "ACK"
 * - Otherwise       -> Reply "ERR"
 *
 * RX is interrupt-driven to avoid losing bytes at 115200 baud.
 */

#define DUT_UART_NODE DT_NODELABEL(usart2)

#define RX_BUFFER_SIZE 64

static const struct device *uart_dev = DEVICE_DT_GET(DUT_UART_NODE);

static char rx_buffer[RX_BUFFER_SIZE];
static size_t rx_index;

static char command_buffer[RX_BUFFER_SIZE];
static volatile bool command_ready;

static void uart_send_string(const char *str)
{
    while (*str != '\0') {
        uart_poll_out(uart_dev, *str);
        str++;
    }
}

static void process_command(const char *cmd)
{
    if (strcmp(cmd, "PING") == 0) {
        uart_send_string("PONG\n");
    } else if (strcmp(cmd, "HELLO") == 0) {
        uart_send_string("ACK\n");
    } else {
        uart_send_string("ERR\n");
    }
}

static void uart_rx_callback(const struct device *dev, void *user_data)
{
    (void)user_data;

    if (!uart_irq_update(dev)) {
        return;
    }

    while (uart_irq_rx_ready(dev)) {
        unsigned char c;

        int ret = uart_fifo_read(dev, &c, 1);
        if (ret <= 0) {
            break;
        }

        if (c == '\r' || c == '\n') {
            if (rx_index == 0) {
                continue;
            }

            rx_buffer[rx_index] = '\0';

            if (!command_ready) {
                strncpy(command_buffer, rx_buffer, sizeof(command_buffer) - 1);
                command_buffer[sizeof(command_buffer) - 1] = '\0';
                command_ready = true;
            }

            rx_index = 0;
            continue;
        }

        if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index] = (char)c;
            rx_index++;
        } else {
            rx_index = 0;
        }
    }
}

int uart_app_init(void)
{
    int ret;

    if (!device_is_ready(uart_dev)) {
        printk("ERROR: USART2 device is not ready\n");
        return -1;
    }

    rx_index = 0;
    command_ready = false;
    memset(rx_buffer, 0, sizeof(rx_buffer));
    memset(command_buffer, 0, sizeof(command_buffer));

    ret = uart_irq_callback_user_data_set(uart_dev, uart_rx_callback, NULL);
    if (ret < 0) {
        printk("ERROR: Failed to set UART RX callback, ret=%d\n", ret);
        return ret;
    }

    uart_irq_rx_enable(uart_dev);

    printk("UART app initialized: USART2 PA2 TX / PA3 RX\n");

    return 0;
}

void uart_app_update(void)
{
    char local_command[RX_BUFFER_SIZE];

    if (!command_ready) {
        return;
    }

    unsigned int key = irq_lock();

    strncpy(local_command, command_buffer, sizeof(local_command) - 1);
    local_command[sizeof(local_command) - 1] = '\0';

    command_ready = false;
    command_buffer[0] = '\0';

    irq_unlock(key);

    process_command(local_command);
}
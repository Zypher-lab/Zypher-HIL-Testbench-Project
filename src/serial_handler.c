#include "serial_handler.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

static const struct device *uart = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static char     rx_buf[SERIAL_BUF_SIZE];    /* accumulate bytes here        */
static char     frame_buf[SERIAL_BUF_SIZE]; /* complete frame stored here   */
static uint16_t rx_pos   = 0;
static bool     frame_ready = false;        /* set by ISR, cleared by main  */

/* ─────────────────────────────────────────────────────────────
 * UART interrupt — runs in interrupt context
 * ONLY accumulates bytes and sets a flag
 * does NOT call any blocking functions
 * ───────────────────────────────────────────────────────────── */
static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t byte;

    if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev)) {
        return;
    }

    while (uart_fifo_read(dev, &byte, 1) == 1) {

        if (byte == '\r') continue;

        if (byte == '\n') {
            rx_buf[rx_pos] = '\0';

            /* only accept frame if we are not already processing one */
            if (rx_pos > 0 && !frame_ready) {
                memcpy(frame_buf, rx_buf, rx_pos + 1);
                frame_ready = true;
            }

            rx_pos = 0;
            return;
        }

        if (rx_pos < SERIAL_BUF_SIZE - 1) {
            rx_buf[rx_pos++] = byte;
        } else {
            rx_pos = 0;
        }
    }
}

void serial_handler_init(void)
{
    if (!device_is_ready(uart)) return;
    uart_irq_callback_set(uart, uart_cb);
    uart_irq_rx_enable(uart);
}

/* ─────────────────────────────────────────────────────────────
 * poll from main loop — safe to call anytime
 * ───────────────────────────────────────────────────────────── */
bool serial_frame_ready(void)
{
    return frame_ready;
}

const char *serial_get_frame(void)
{
    frame_ready = false;    /* clear flag — ready for next frame */
    return frame_buf;
}

void serial_send(const char *msg)
{
    while (*msg != '\0') {
        uart_poll_out(uart, *msg++);
    }
    uart_poll_out(uart, '\n');
}
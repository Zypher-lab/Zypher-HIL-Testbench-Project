
#include "test_executor.h"

#include "gpio_service.h"
#include "dac_service.h"
#include "uart_service.h"

#include <string.h>

static void execute_gpio_write(const ztb_command_t *command,
                               ztb_response_t *response)
{
    if (!gpio_service_write(command->ch, command->val)) {
        ztb_make_fail_response(response, command->seq, "GPIO_WRITE_FAILED");
        return;
    }

    ztb_make_ok_response(response, command->seq);
}

static void execute_gpio_read(const ztb_command_t *command,
                              ztb_response_t *response)
{
    int val;

    if (!gpio_service_read(command->ch, &val)) {
        ztb_make_fail_response(response, command->seq, "GPIO_READ_FAILED");
        return;
    }

    ztb_make_gpio_value_response(response, command->seq, val);
}

static void execute_gpio_expect(const ztb_command_t *command,
                                ztb_response_t *response)
{
    int actual;

    bool pass = gpio_service_expect(command->ch,
                                    command->val,
                                    command->timeout_ms,
                                    &actual);

    ztb_make_gpio_expect_response(response,
                                  command->seq,
                                  pass,
                                  command->val,
                                  actual);
}

static void execute_dac_write(const ztb_command_t *command,
                              ztb_response_t *response)
{
    if (!dac_service_write_mv(command->ch, command->mv)) {
        ztb_make_fail_response(response, command->seq, "DAC_WRITE_FAILED");
        return;
    }

    ztb_make_ok_response(response, command->seq);
}

static void execute_uart_write(const ztb_command_t *command,
                               ztb_response_t *response)
{
    if (!uart_service_write(command->ch, command->tx)) {
        ztb_make_fail_response(response, command->seq, "UART_WRITE_FAILED");
        return;
    }

    ztb_make_ok_response(response, command->seq);
}

static void execute_uart_read(const ztb_command_t *command,
                              ztb_response_t *response)
{
    char rx_buffer[64];

    if (!uart_service_read(command->ch,
                           command->timeout_ms,
                           rx_buffer,
                           sizeof(rx_buffer))) {
        ztb_make_fail_response(response, command->seq, "UART_READ_TIMEOUT");
        return;
    }

    ztb_make_uart_rx_response(response, command->seq, rx_buffer);
}

static void execute_uart_send_expect(const ztb_command_t *command,
                                     ztb_response_t *response)
{
    char rx_buffer[64];

    if (!uart_service_send_expect(command->ch,
                                  command->tx,
                                  command->expect,
                                  command->timeout_ms,
                                  rx_buffer,
                                  sizeof(rx_buffer))) {
        ztb_make_fail_response(response, command->seq, "UART_MISMATCH");
        response->has_rx = true;
        strncpy(response->rx, rx_buffer, sizeof(response->rx) - 1);
        return;
    }

    ztb_make_uart_rx_response(response, command->seq, rx_buffer);
}

void test_executor_execute(const ztb_command_t *command,
                           ztb_response_t *response)
{
    switch (command->cmd) {
    case ZTB_CMD_GPIO_WRITE:
        execute_gpio_write(command, response);
        break;

    case ZTB_CMD_GPIO_READ:
        execute_gpio_read(command, response);
        break;

    case ZTB_CMD_GPIO_EXPECT:
        execute_gpio_expect(command, response);
        break;

    case ZTB_CMD_DAC_WRITE:
        execute_dac_write(command, response);
        break;

    case ZTB_CMD_UART_WRITE:
        execute_uart_write(command, response);
        break;

    case ZTB_CMD_UART_READ:
        execute_uart_read(command, response);
        break;

    case ZTB_CMD_UART_SEND_EXPECT:
        execute_uart_send_expect(command, response);
        break;

    default:
        ztb_make_fail_response(response, command->seq, "UNKNOWN_COMMAND");
        break;
    }
}
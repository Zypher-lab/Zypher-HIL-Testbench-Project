#include <zephyr/kernel.h>

#include "uart_transport.h"
#include "ztb_protocol.h"
#include "test_executor.h"
#include "gpio_service.h"

#define LINE_BUF_SIZE 128
#define RESPONSE_BUF_SIZE 160

int main(void)
{
    char line[LINE_BUF_SIZE];
    char response_line[RESPONSE_BUF_SIZE];

    ztb_command_t command;
    ztb_response_t response;

    if (uart_transport_init() != 0) {
        return 0;
    }

    if (gpio_service_init() != 0) {
        uart_transport_send_line("ZTB|status=FAIL|err=GPIO_INIT_FAILED\r\n");
        return 0;
    }

    uart_transport_send_line("ZTB|status=READY\r\n");

    while (1) {
        uart_transport_read_line(line, sizeof(line));

        if (!ztb_parse_frame(line, &command)) {
            ztb_make_fail_response(&response, -1, "PARSE_ERROR");
        } else {
            test_executor_execute(&command, &response);
        }

        ztb_format_response(&response, response_line, sizeof(response_line));
        uart_transport_send_line(response_line);
    }

    return 0;
}
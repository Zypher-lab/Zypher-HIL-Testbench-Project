#include <zephyr/kernel.h>
#include "wifi_transport.h"       /* ← only change from uart version */
#include "ztb_protocol.h"
#include "test_executor.h"
#include "gpio_service.h"
#include "dac_service.h"
#include "uart_service.h"
#include "pwm_service.h"
#include "spi_service.h"

#define LINE_BUF_SIZE     128
#define RESPONSE_BUF_SIZE 160

int main(void)
{
    char line[LINE_BUF_SIZE];
    char response_line[RESPONSE_BUF_SIZE];
    ztb_command_t command;
    ztb_response_t response;

    if (wifi_transport_init() != 0) return 0;

    if (gpio_service_init() != 0) {
        wifi_transport_send_line("ZTB|status=FAIL|err=GPIO_INIT_FAILED\r\n");
        return 0;
    }
    if (dac_service_init() != 0) {
        wifi_transport_send_line("ZTB|status=FAIL|err=DAC_INIT_FAILED\r\n");
        return 0;
    }
    if (uart_service_init() != 0) {
        wifi_transport_send_line("ZTB|status=FAIL|err=UART_INIT_FAILED\r\n");
        return 0;
    }
    if (pwm_service_init() != 0) {
        wifi_transport_send_line("ZTB|status=FAIL|err=PWM_INIT_FAILED\r\n");
        return 0;
    }
    if (spi_service_init() != 0) {
        wifi_transport_send_line("ZTB|status=FAIL|err=SPI_INIT_FAILED\r\n");
        return 0;
    }

    wifi_transport_send_line("ZTB|status=READY\r\n");

    while (1) {
        wifi_transport_read_line(line, sizeof(line));

        if (!ztb_parse_frame(line, &command)) {
            ztb_make_fail_response(&response, -1, "PARSE_ERROR");
        } else {
            test_executor_execute(&command, &response);
        }

        ztb_format_response_with_cmd(&response, &command,
                                     response_line, sizeof(response_line));
        wifi_transport_send_line(response_line);
    }

    return 0;
}
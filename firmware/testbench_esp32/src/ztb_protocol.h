#ifndef ZTB_PROTOCOL_H
#define ZTB_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    ZTB_CMD_UNKNOWN = 0,
    ZTB_CMD_GPIO_WRITE,
    ZTB_CMD_GPIO_READ,
    ZTB_CMD_GPIO_EXPECT,
    ZTB_CMD_DAC_WRITE,
    ZTB_CMD_UART_WRITE,
    ZTB_CMD_UART_READ,
    ZTB_CMD_UART_SEND_EXPECT,
    ZTB_CMD_PWM_WRITE,
    ZTB_CMD_PWM_READ_WITH_TOLERANCE,
    ZTB_CMD_SPI_WRITE,
    ZTB_CMD_SPI_SEND_EXPECT,
    ZTB_CMD_TRANSPORT_SWITCH,       /* mode=UART or mode=WIFI → reboot */
} ztb_command_type_t;

typedef enum {
    ZTB_STATUS_OK = 0,
    ZTB_STATUS_FAIL,
} ztb_status_t;

typedef struct {
    int                seq;
    ztb_command_type_t cmd;
    char               ch[24];      /* channel name OR mode for TRANSPORT_SWITCH */
    int                val;
    int                mv;
    int                timeout_ms;
    int                duty_cycle;
    int                frequency;
    int                freq_tol_pct;
    int                duty_tol_pp;
    char               tx[64];
    char               expect[64];
} ztb_command_t;

typedef struct {
    int          seq;
    ztb_status_t status;
    bool         has_val;
    int          val;
    int          duty_cycle_expected;
    int          frequency_expected;
    int          duty_cycle_measured;
    int          frequency_measured;
    int          freq_tol_pct;
    int          duty_tol_pp;
    bool         has_expected_actual;
    int          expected;
    int          actual;
    bool         has_rx;
    char         rx[64];
    char         err[48];
} ztb_response_t;

bool ztb_parse_frame(char *line, ztb_command_t *out_cmd);

void ztb_format_response(const ztb_response_t *response,
                         char *out_line, size_t out_size);

void ztb_format_response_with_cmd(const ztb_response_t *response,
                                  const ztb_command_t  *cmd,
                                  char *out_line, size_t out_size);

void ztb_make_ok_response(ztb_response_t *response, int seq);
void ztb_make_fail_response(ztb_response_t *response, int seq, const char *err);
void ztb_make_gpio_value_response(ztb_response_t *response, int seq, int val);
void ztb_make_gpio_expect_response(ztb_response_t *response, int seq, bool pass,
                                   int expected, int actual);
void ztb_make_uart_rx_response(ztb_response_t *response, int seq, const char *rx);
void ztb_make_pwm_write_response(ztb_response_t *response, int seq, bool pass,
                                 int duty_cycle_set, int frequency_set);
void ztb_make_pwm_expect_response(ztb_response_t *response, int seq, bool pass,
                                  int duty_cycle_expected, int frequency_expected,
                                  int duty_cycle_measured, int frequency_measured,
                                  int freq_tol_pct, int duty_tol_pp);

#endif
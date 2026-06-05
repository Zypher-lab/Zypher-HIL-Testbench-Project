
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
} ztb_command_type_t;

typedef enum {
    ZTB_STATUS_OK = 0,
    ZTB_STATUS_FAIL,
} ztb_status_t;

typedef struct {
    int seq;
    ztb_command_type_t cmd;
    char ch[24];

    int val;
    int mv;
    int timeout_ms;

    char tx[64];
    char expect[64];
} ztb_command_t;

typedef struct {
    int seq;
    ztb_status_t status;

    bool has_val;
    int val;

    bool has_expected_actual;
    int expected;
    int actual;

    bool has_rx;
    char rx[64];

    char err[48];
} ztb_response_t;

bool ztb_parse_frame(char *line, ztb_command_t *out_cmd);

void ztb_make_ok_response(ztb_response_t *response, int seq);
void ztb_make_fail_response(ztb_response_t *response, int seq, const char *err);

void ztb_make_gpio_value_response(ztb_response_t *response, int seq, int val);

void ztb_make_gpio_expect_response(ztb_response_t *response,
                                   int seq,
                                   bool pass,
                                   int expected,
                                   int actual);

void ztb_make_uart_rx_response(ztb_response_t *response,
                               int seq,
                               const char *rx);

void ztb_format_response(const ztb_response_t *response,
                         char *out_line,
                         size_t out_size);

#endif
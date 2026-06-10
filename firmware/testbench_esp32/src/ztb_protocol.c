
#include "ztb_protocol.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static ztb_command_type_t parse_command_name(const char *cmd)
{
    if (strcmp(cmd, "GPIO_WRITE") == 0) {
        return ZTB_CMD_GPIO_WRITE;
    }

    if (strcmp(cmd, "GPIO_READ") == 0) {
        return ZTB_CMD_GPIO_READ;
    }

    if (strcmp(cmd, "GPIO_EXPECT") == 0) {
        return ZTB_CMD_GPIO_EXPECT;
    }

    if (strcmp(cmd, "DAC_WRITE") == 0) {
        return ZTB_CMD_DAC_WRITE;
    }

    if (strcmp(cmd, "UART_WRITE") == 0) {
        return ZTB_CMD_UART_WRITE;
    }

    if (strcmp(cmd, "UART_READ") == 0) {
        return ZTB_CMD_UART_READ;
    }

    if (strcmp(cmd, "UART_SEND_EXPECT") == 0) {
        return ZTB_CMD_UART_SEND_EXPECT;
    }
    
     /*My add*/
    if (strcmp(cmd, "PWM_WRITE") == 0) {
        return ZTB_CMD_PWM_WRITE;
    }
    if (strcmp(cmd, "PWM_READ_WITH_TOLERANCE") == 0) {
        return ZTB_CMD_PWM_READ_WITH_TOLERANCE;
    }
    if (strcmp(cmd, "SPI_SEND_EXPECT") == 0) {
        return ZTB_CMD_SPI_SEND_EXPECT;
    }
    if (strcmp(cmd, "SPI_WRITE") == 0) {
        return ZTB_CMD_SPI_WRITE;
    }

    return ZTB_CMD_UNKNOWN;
}

bool ztb_parse_frame(char *line, ztb_command_t *out_cmd)
{
    memset(out_cmd, 0, sizeof(*out_cmd));

    out_cmd->seq = -1;
    out_cmd->cmd = ZTB_CMD_UNKNOWN;
    out_cmd->timeout_ms = 100;

    char *saveptr = NULL;
    char *token = strtok_r(line, "|", &saveptr);

    if (token == NULL || strcmp(token, "ZTB") != 0) {
        return false;
    }

    while ((token = strtok_r(NULL, "|", &saveptr)) != NULL) {
        char *eq = strchr(token, '=');

        if (eq == NULL) {
            continue;
        }

        *eq = '\0';

        const char *key = token;
        const char *value = eq + 1;

        if (strcmp(key, "seq") == 0) {
            out_cmd->seq = atoi(value);
        } else if (strcmp(key, "cmd") == 0) {
            out_cmd->cmd = parse_command_name(value);
        } else if (strcmp(key, "ch") == 0) {
            strncpy(out_cmd->ch, value, sizeof(out_cmd->ch) - 1);
        } else if (strcmp(key, "val") == 0) {
            out_cmd->val = atoi(value);
        } else if (strcmp(key, "timeout") == 0) {
            out_cmd->timeout_ms = atoi(value);
        } else if (strcmp(key, "mv") == 0) {
            out_cmd->mv = atoi(value);
        } else if (strcmp(key, "tx") == 0) {
            strncpy(out_cmd->tx, value, sizeof(out_cmd->tx) - 1);
        } else if (strcmp(key, "expect") == 0) {
            strncpy(out_cmd->expect, value, sizeof(out_cmd->expect) - 1);
        }
        else if (strcmp(key, "duty_cycle") == 0) {
            out_cmd->duty_cycle = atoi(value);
        }else if (strcmp(key, "frequency") == 0) {
            out_cmd->frequency = atoi(value);
        }
        else if (strcmp(key, "freq_tol_pct") == 0) {
            out_cmd->freq_tol_pct = atoi(value);
        }
        else if (strcmp(key, "duty_tol_pp") == 0) {
            out_cmd->duty_tol_pp = atoi(value);
        }
    }

    if (out_cmd->seq < 0) {
        return false;
    }

    if (out_cmd->cmd == ZTB_CMD_UNKNOWN) {
        return false;
    }

    return true;
}

void ztb_make_ok_response(ztb_response_t *response, int seq)
{
    memset(response, 0, sizeof(*response));

    response->seq = seq;
    response->status = ZTB_STATUS_OK;
}

void ztb_make_fail_response(ztb_response_t *response, int seq, const char *err)
{
    memset(response, 0, sizeof(*response));

    response->seq = seq;
    response->status = ZTB_STATUS_FAIL;

    strncpy(response->err, err, sizeof(response->err) - 1);
}

void ztb_make_gpio_value_response(ztb_response_t *response, int seq, int val)
{
    ztb_make_ok_response(response, seq);

    response->has_val = true;
    response->val = val ? 1 : 0;
}

void ztb_make_gpio_expect_response(ztb_response_t *response,
                                   int seq,
                                   bool pass,
                                   int expected,
                                   int actual)
{
    memset(response, 0, sizeof(*response));

    response->seq = seq;
    response->status = pass ? ZTB_STATUS_OK : ZTB_STATUS_FAIL;

    response->has_expected_actual = true;
    response->expected = expected ? 1 : 0;
    response->actual = actual ? 1 : 0;

    if (!pass) {
        strncpy(response->err, "GPIO_MISMATCH", sizeof(response->err) - 1);
    }
}

void ztb_make_uart_rx_response(ztb_response_t *response,
                               int seq,
                               const char *rx)
{
    ztb_make_ok_response(response, seq);

    response->has_rx = true;
    strncpy(response->rx, rx, sizeof(response->rx) - 1);
}

void ztb_format_response(const ztb_response_t *response,
                         char *out_line,
                         size_t out_size)
{
    if (response->status == ZTB_STATUS_OK) {
        if (response->has_expected_actual) {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=OK|expected=%d|actual=%d\r\n",
                     response->seq,
                     response->expected,
                     response->actual);
        } else if (response->has_val) {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=OK|val=%d\r\n",
                     response->seq,
                     response->val);
        } else if (response->has_rx) {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=OK|rx=%s\r\n",
                     response->seq,
                     response->rx);
        }else  {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=OK\r\n",
                     response->seq);
        }

    } else {
        if (response->has_expected_actual) {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=FAIL|expected=%d|actual=%d|err=%s\r\n",
                     response->seq,
                     response->expected,
                     response->actual,
                     response->err);
        } else if (response->has_rx) {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=FAIL|rx=%s|err=%s\r\n",
                     response->seq,
                     response->rx,
                     response->err);
        } 
        else {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=FAIL|err=%s\r\n",
                     response->seq,
                     response->err);
        }
    }
}



/* PWM response functions */
void ztb_make_pwm_write_response(ztb_response_t *response,
                                 int seq, bool pass,
                                 int duty_cycle_set, int frequency_set)
{
    memset(response, 0, sizeof(*response));
    response->seq                 = seq;
    response->status              = pass ? ZTB_STATUS_OK : ZTB_STATUS_FAIL;
    response->duty_cycle_expected = duty_cycle_set;
    response->frequency_expected  = frequency_set;
    if (!pass) strncpy(response->err, "PWM_WRITE_FAIL", sizeof(response->err) - 1);
}
 
void ztb_make_pwm_read_response(ztb_response_t *response,
                                int seq, bool pass,
                                int duty_cycle_measured, int frequency_measured)
{
    memset(response, 0, sizeof(*response));
    response->seq                 = seq;
    response->status              = pass ? ZTB_STATUS_OK : ZTB_STATUS_FAIL;
    response->duty_cycle_measured = duty_cycle_measured;
    response->frequency_measured  = frequency_measured;
    if (!pass) strncpy(response->err, "PWM_READ_FAIL", sizeof(response->err) - 1);
}
 
void ztb_make_pwm_read_with_tolerance_response(ztb_response_t *response,
                                  int seq, bool pass,
                                  int duty_cycle_expected, int frequency_expected,
                                  int duty_cycle_measured, int frequency_measured,
                                  int freq_tol_pct, int duty_tol_pp)
{
    memset(response, 0, sizeof(*response));
    response->seq                 = seq;
    response->status              = pass ? ZTB_STATUS_OK : ZTB_STATUS_FAIL;
    response->duty_cycle_expected = duty_cycle_expected;
    response->frequency_expected  = frequency_expected;
    response->duty_cycle_measured = duty_cycle_measured;
    response->frequency_measured  = frequency_measured;
    response->freq_tol_pct        = freq_tol_pct;
    response->duty_tol_pp         = duty_tol_pp;
    if (!pass) strncpy(response->err, "PWM_MISMATCH", sizeof(response->err) - 1);
}

void ztb_format_response_with_cmd(const ztb_response_t *response,
                                  const ztb_command_t  *cmd,
                                  char *out_line, size_t out_size)
{
    if (cmd->cmd != ZTB_CMD_PWM_WRITE &&
        cmd->cmd != ZTB_CMD_PWM_READ_WITH_TOLERANCE) {
        ztb_format_response(response, out_line, out_size);
        return;
    }
 
    if (response->status == ZTB_STATUS_OK) {
        if (cmd->cmd == ZTB_CMD_PWM_READ_WITH_TOLERANCE) {
            snprintf(out_line, out_size,
                "ZTB|seq=%d|status=OK"
                "|freq_expected=%d|duty_expected=%d"
                "|freq_measured=%d|duty_measured=%d"
                "|freq_tol_pct=%d|duty_tol_pp=%d\r\n",
                response->seq,
                response->frequency_expected,  response->duty_cycle_expected,
                response->frequency_measured,  response->duty_cycle_measured,
                response->freq_tol_pct,        response->duty_tol_pp);
        } else {
            snprintf(out_line, out_size,
                "ZTB|seq=%d|status=OK|freq_set=%d|duty_set=%d\r\n",
                response->seq,
                response->frequency_expected,
                response->duty_cycle_expected);
        }
    } else {
        if (cmd->cmd == ZTB_CMD_PWM_READ_WITH_TOLERANCE) {
            snprintf(out_line, out_size,
                "ZTB|seq=%d|status=FAIL"
                "|freq_expected=%d|duty_expected=%d"
                "|freq_measured=%d|duty_measured=%d"
                "|freq_tol_pct=%d|duty_tol_pp=%d|err=%s\r\n",
                response->seq,
                response->frequency_expected,  response->duty_cycle_expected,
                response->frequency_measured,  response->duty_cycle_measured,
                response->freq_tol_pct,        response->duty_tol_pp,
                response->err);
        } else {
            snprintf(out_line, out_size,
                "ZTB|seq=%d|status=FAIL|freq_set=%d|duty_set=%d|err=%s\r\n",
                response->seq,
                response->frequency_expected,
                response->duty_cycle_expected,
                response->err);
        }
    }
}

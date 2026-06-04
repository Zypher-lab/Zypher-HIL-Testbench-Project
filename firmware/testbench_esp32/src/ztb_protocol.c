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
        } else {
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
        } else {
            snprintf(out_line, out_size,
                     "ZTB|seq=%d|status=FAIL|err=%s\r\n",
                     response->seq,
                     response->err);
        }
    }
}
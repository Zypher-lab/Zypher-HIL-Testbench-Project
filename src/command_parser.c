#include "command_parser.h"
#include <string.h>
#include <stdlib.h>

/* working buffer — we copy the frame here before splitting
 * because strtok destroys the string it works on            */
static char buf[256];

/* ─────────────────────────────────────────────────────────────
 * convert action string to enum
 * input : "PWM" / "GPIO" / "UART" / "SPI" / "ADC"
 * output: matching action_t value, or ACTION_UNKNOWN
 * ───────────────────────────────────────────────────────────── */
static action_t parse_action(const char *str)
{
    if (strcmp(str, "PWM")  == 0) return ACTION_PWM;
    if (strcmp(str, "GPIO") == 0) return ACTION_GPIO;
    if (strcmp(str, "UART") == 0) return ACTION_UART;
    if (strcmp(str, "SPI")  == 0) return ACTION_SPI;
    if (strcmp(str, "ADC")  == 0) return ACTION_ADC;

    return ACTION_UNKNOWN;
}

/* ─────────────────────────────────────────────────────────────
 * parse one operation from a colon-separated token
 *
 * example input : "PWM:PA8:50:90"
 * example output: op->action   = ACTION_PWM
 *                 op->pin      = "PA8"
 *                 op->num_params  = {50, 90}
 *                 op->num_param_count = 2
 *
 * rule: if param starts with digit  → goes to num_params
 *       if param starts with letter → goes to str_params
 * ───────────────────────────────────────────────────────────── */
static parse_result_t parse_operation(char *token, operation_t *op)
{
    char *field;
    char *save;     /* own strtok_r context — must not share with callers */

    /* zero the operation before filling it */
    memset(op, 0, sizeof(operation_t));

    /* ── step 1: action ── */
    field = strtok_r(token, ":", &save);
    if (!field) return PARSE_ERR_BAD_FRAME;

    op->action = parse_action(field);
    if (op->action == ACTION_UNKNOWN) return PARSE_ERR_BAD_ACTION;

    /* ── step 2: pin ── */
    field = strtok_r(NULL, ":", &save);
    if (!field) return PARSE_ERR_BAD_FRAME;

    strncpy(op->pin, field, MAX_PIN_LEN - 1);

    /* ── step 3: remaining params ── */
    field = strtok_r(NULL, ":", &save);

    while (field != NULL) {

        if (field[0] >= '0' && field[0] <= '9') {
            /* numeric param — e.g. "50", "90", "1" */
            op->num_params[op->num_param_count] = (uint32_t)atoi(field);
            op->num_param_count++;

        } else {
            /* string param — e.g. "forward", "0x55" */
            strncpy(op->str_params[op->str_param_count],
                    field,
                    MAX_STR_PARAM_LEN - 1);
            op->str_param_count++;
        }

        /* get next param */
        field = strtok_r(NULL, ":", &save);
    }

    return PARSE_OK;
}

/* ─────────────────────────────────────────────────────────────
 * parse a block of comma-separated operations
 *
 * example input : "GPIO:PB10:1,GPIO:PB9:1"
 * example output: ops[0] = {GPIO, PB10, num={1}}
 *                 ops[1] = {GPIO, PB9,  num={1}}
 *                 count  = 2
 * ───────────────────────────────────────────────────────────── */
static parse_result_t parse_block(char *block, operation_t *ops, uint8_t *count)
{
    parse_result_t res;
    char *token;
    char *save;     /* own strtok_r context — independent of parse_operation */

    *count = 0;

    /* split block by comma — each piece is one operation */
    token = strtok_r(block, ",", &save);

    while (token != NULL) {

        /* safety check — don't overflow ops array */
        if (*count >= MAX_OPS) return PARSE_ERR_TOO_MANY_OPS;

        /* parse this operation into the next slot */
        res = parse_operation(token, &ops[*count]);
        if (res != PARSE_OK) return res;

        (*count)++;

        /* move to next operation */
        token = strtok_r(NULL, ",", &save);
    }

    return PARSE_OK;
}

/* ─────────────────────────────────────────────────────────────
 * main parse function — only function called from outside
 *
 * full frame format:
 * "SET:<op>,<op>|ASSERT:<op>,<op>|TIMEOUT:<ms>"
 *
 * examples:
 * "SET:PWM:PA8:50:90|ASSERT:PWM:PB10:50:90|TIMEOUT:1000"
 * "SET:GPIO:PB10:1,GPIO:PB9:1|ASSERT:GPIO:PB11:1|TIMEOUT:500"
 * ───────────────────────────────────────────────────────────── */
parse_result_t command_parse(const char *frame, parsed_cmd_t *out)
{
    char *section;
    char *save;     /* own strtok_r context for the top-level '|' split */

    /* zero the output struct before filling it */
    memset(out, 0, sizeof(parsed_cmd_t));

    /* reject empty frame */
    if (!frame || frame[0] == '\0') return PARSE_ERR_EMPTY;

    /* copy frame into working buffer */
    strncpy(buf, frame, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
        buf[len-1] = '\0';
    }

    /* split full frame by '|' to get sections
     * section 0 → "SET:..."
     * section 1 → "ASSERT:..."
     * section 2 → "TIMEOUT:..."         */
    section = strtok_r(buf, "|", &save);

    while (section != NULL) {

        if (strncmp(section, "SET:", 4) == 0) {
            /* skip past "SET:" and parse the operations block */
            parse_block(section + 4, out->set_ops, &out->set_count);

        } else if (strncmp(section, "ASSERT:", 7) == 0) {
            /* skip past "ASSERT:" and parse the operations block */
            parse_block(section + 7, out->assert_ops, &out->assert_count);

        } else if (strncmp(section, "TIMEOUT:", 8) == 0) {
            /* skip past "TIMEOUT:" and read the number */
            out->timeout_ms = (uint32_t)atoi(section + 8);

        } else {
            return PARSE_ERR_BAD_FRAME;
        }

        /* move to next section */
        section = strtok_r(NULL, "|", &save);
    }

    return PARSE_OK;
}

/* ─────────────────────────────────────────────────────────────
 * convert result code to string for UART response
 * ───────────────────────────────────────────────────────────── */
const char *parse_result_str(parse_result_t result)
{
    switch (result) {
        case PARSE_OK:               return "OK";
        case PARSE_ERR_EMPTY:        return "FAIL:EMPTY_FRAME";
        case PARSE_ERR_BAD_ACTION:   return "FAIL:BAD_ACTION";
        case PARSE_ERR_BAD_FRAME:    return "FAIL:BAD_FRAME";
        case PARSE_ERR_OUT_OF_RANGE: return "FAIL:OUT_OF_RANGE";
        case PARSE_ERR_TOO_MANY_OPS: return "FAIL:TOO_MANY_OPS";
        default:                     return "FAIL:UNKNOWN";
    }
}
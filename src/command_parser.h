#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdint.h>

/* ─── Limits ─────────────────────────────────────────────────────────────── */
#define MAX_PIN_LEN         8       /* "PA8", "PB10" etc                     */
#define MAX_STR_PARAMS      4       /* max string params per operation        */
#define MAX_STR_PARAM_LEN   32      /* max length of each string param        */
#define MAX_NUM_PARAMS      4       /* max numeric params per operation       */
#define MAX_OPS             8       /* max operations per SET or ASSERT block */

/* ─── Action types ───────────────────────────────────────────────────────── */
typedef enum {
    ACTION_UNKNOWN = 0,
    ACTION_PWM,
    ACTION_GPIO,
    ACTION_UART,
    ACTION_SPI,
    ACTION_ADC,
} action_t;

/* ─── Single operation ───────────────────────────────────────────────────── */
/*
 * One action on one pin with its parameters.
 *
 * How each action uses params:
 *
 *   PWM:
 *     num_params[0] = frequency (Hz)
 *     num_params[1] = duty cycle (%)
 *
 *   GPIO:
 *     num_params[0] = level (0 or 1)
 *
 *   UART:
 *     str_params[0] = string to send e.g. "forward"
 *     str_params[1] = string to expect (assert side)
 *
 *   SPI:
 *     str_params[0] = hex bytes to send e.g. "0x55"
 *     str_params[1] = hex bytes to expect (assert side)
 *
 *   ADC:
 *     num_params[0] = expected voltage in mV
 */
typedef struct {
    action_t    action;
    char        pin[MAX_PIN_LEN];

    uint32_t    num_params[MAX_NUM_PARAMS];
    uint8_t     num_param_count;

    char        str_params[MAX_STR_PARAMS][MAX_STR_PARAM_LEN];
    uint8_t     str_param_count;
} operation_t;

/* ─── Full parsed command ────────────────────────────────────────────────── */
/*
 * Holds all SET operations and all ASSERT operations from one frame.
 *
 * Example frame:
 * "SET:GPIO:PB10:1,GPIO:PB9:1,PWM:PA8:50:90|ASSERT:PWM:PB10:50:90,GPIO:PB11:1|TIMEOUT:1000"
 *
 * Results in:
 *   set_ops[0]    = {ACTION_GPIO, "PB10", num_params={1}}
 *   set_ops[1]    = {ACTION_GPIO, "PB9",  num_params={1}}
 *   set_ops[2]    = {ACTION_PWM,  "PA8",  num_params={50, 90}}
 *   set_count     = 3
 *
 *   assert_ops[0] = {ACTION_PWM,  "PB10", num_params={50, 90}}
 *   assert_ops[1] = {ACTION_GPIO, "PB11", num_params={1}}
 *   assert_count  = 2
 *
 *   timeout_ms    = 1000
 */
typedef struct {
    operation_t set_ops[MAX_OPS];
    uint8_t     set_count;

    operation_t assert_ops[MAX_OPS];
    uint8_t     assert_count;

    uint32_t    timeout_ms;
} parsed_cmd_t;

/* ─── Return codes ───────────────────────────────────────────────────────── */
typedef enum {
    PARSE_OK = 0,
    PARSE_ERR_EMPTY,
    PARSE_ERR_BAD_ACTION,
    PARSE_ERR_BAD_FRAME,
    PARSE_ERR_OUT_OF_RANGE,
    PARSE_ERR_TOO_MANY_OPS,
} parse_result_t;

/* ─── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Parse a raw UART frame into a command struct.
 *
 * @param  frame      Null-terminated string received over UART.
 *
 *   Format:
 *   "SET:<op1>,<op2>,...|ASSERT:<op1>,<op2>,...|TIMEOUT:<ms>"
 *
 *   Each operation: <ACTION>:<PIN>:<param1>:<param2>...
 *
 *   Examples:
 *   "SET:PWM:PA8:50:90|ASSERT:PWM:PB10:50:90|TIMEOUT:1000"
 *   "SET:GPIO:PB10:1,GPIO:PB9:1|ASSERT:GPIO:PB11:1,GPIO:PB12:0|TIMEOUT:500"
 *   "SET:UART:PA9:forward|ASSERT:GPIO:PB11:1,GPIO:PB12:0|TIMEOUT:500"
 *   "SET:GPIO:PB10:1,GPIO:PB9:1,PWM:PA8:50:90|ASSERT:PWM:PB10:50:90|TIMEOUT:1000"
 *
 * @param  out        Pointer to struct to fill. Cleared before parsing.
 * @return parse_result_t
 */
parse_result_t command_parse(const char *frame, parsed_cmd_t *out);

/**
 * @brief  Convert a parse result code to a human readable string.
 */
const char *parse_result_str(parse_result_t result);

#endif /* COMMAND_PARSER_H */
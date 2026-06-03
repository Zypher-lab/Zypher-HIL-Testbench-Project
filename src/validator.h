#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdint.h>
#include "command_parser.h"

/* ─── Validation result ──────────────────────────────────────────────────── */
typedef enum {
    VALIDATE_OK = 0,
    VALIDATE_FAIL_FREQ,         /* measured frequency does not match expected */
    VALIDATE_FAIL_DUTY,         /* measured duty cycle does not match expected */
    VALIDATE_FAIL_GPIO,         /* pin level does not match expected           */
    VALIDATE_FAIL_UART,         /* uart response does not match expected       */
    VALIDATE_FAIL_SPI,          /* spi response does not match expected        */
    VALIDATE_FAIL_ADC,          /* adc voltage does not match expected         */
    VALIDATE_FAIL_TIMEOUT,      /* measurement timed out                       */
    VALIDATE_FAIL_UNKNOWN,      /* action not supported                        */
} validate_result_t;

/* ─── Measured values struct ─────────────────────────────────────────────── */
/*
 * Filled by the handler (pwm_capture, gpio_handler etc.)
 * then passed to validate() alongside the expected operation_t
 */
typedef struct {
    uint32_t    num_values[MAX_NUM_PARAMS];     /* measured numeric values   */
    uint8_t     num_value_count;

    char        str_values[MAX_STR_PARAMS][MAX_STR_PARAM_LEN]; /* measured strings */
    uint8_t     str_value_count;
} measured_t;

/* ─── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Compare measured values against one expected operation.
 *
 * @param  expected   The assert operation from parsed_cmd_t.
 * @param  measured   Values read back from hardware.
 * @return validate_result_t
 */
validate_result_t validate(const operation_t *expected, const measured_t *measured);

/**
 * @brief  Convert validation result to response string for UART.
 */
const char *validate_result_str(validate_result_t result);

#endif /* VALIDATOR_H */
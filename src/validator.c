#include "validator.h"
#include <string.h>

/* ─── PWM tolerances ──────────────────────────────────────────
 * Four-channel capture resets the counter in the ISR rather than
 * the slave-mode controller, so the measured period can jitter by
 * a few timer cycles. With exact == that flips integer-divided
 * results (e.g. 1000000/20001 = 49 instead of 50), so we allow a
 * small band instead.
 *
 *   FREQ_TOL_PCT : allowed frequency error, percent of expected
 *   DUTY_TOL_PP  : allowed duty error, absolute percentage points
 * ───────────────────────────────────────────────────────────── */
#define FREQ_TOL_PCT    2u
#define DUTY_TOL_PP     2u

/* absolute difference for unsigned values (no underflow) */
static uint32_t udiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

/* ─────────────────────────────────────────────────────────────
 * main validate function
 *
 * looks at expected->action to know what to compare
 * then compares expected params against measured values
 * ───────────────────────────────────────────────────────────── */
validate_result_t validate(const operation_t *expected, const measured_t *measured)
{
    switch (expected->action) {

        /* ── PWM ──────────────────────────────────────────────
         * num_params[0] = expected frequency (Hz)
         * num_params[1] = expected duty cycle (%)
         * ───────────────────────────────────────────────────── */
        case ACTION_PWM:

            /* check frequency — within FREQ_TOL_PCT % of expected */
            if (udiff(measured->num_values[0], expected->num_params[0]) * 100u
                    > expected->num_params[0] * FREQ_TOL_PCT) {
                return VALIDATE_FAIL_FREQ;
            }

            /* check duty cycle — within DUTY_TOL_PP percentage points */
            if (udiff(measured->num_values[1], expected->num_params[1])
                    > DUTY_TOL_PP) {
                return VALIDATE_FAIL_DUTY;
            }

            return VALIDATE_OK;

        /* ── GPIO ─────────────────────────────────────────────
         * num_params[0] = expected level (0 or 1)
         * ───────────────────────────────────────────────────── */
        case ACTION_GPIO:

            if (measured->num_values[0] != expected->num_params[0]) {
                return VALIDATE_FAIL_GPIO;
            }

            return VALIDATE_OK;

        /* ── UART ─────────────────────────────────────────────
         * str_params[0] = expected string response
         * ───────────────────────────────────────────────────── */
        case ACTION_UART:

            if (strcmp(measured->str_values[0], expected->str_params[0]) != 0) {
                return VALIDATE_FAIL_UART;
            }

            return VALIDATE_OK;

        /* ── SPI ──────────────────────────────────────────────
         * str_params[0] = expected bytes e.g. "0x55"
         * ───────────────────────────────────────────────────── */
        case ACTION_SPI:

            if (strcmp(measured->str_values[0], expected->str_params[0]) != 0) {
                return VALIDATE_FAIL_SPI;
            }

            return VALIDATE_OK;

        /* ── ADC ──────────────────────────────────────────────
         * num_params[0] = expected voltage in mV
         * ───────────────────────────────────────────────────── */
        case ACTION_ADC:

            if (measured->num_values[0] != expected->num_params[0]) {
                return VALIDATE_FAIL_ADC;
            }

            return VALIDATE_OK;

        default:
            return VALIDATE_FAIL_UNKNOWN;
    }
}

/* ─────────────────────────────────────────────────────────────
 * convert result to string for UART response
 * ───────────────────────────────────────────────────────────── */
const char *validate_result_str(validate_result_t result)
{
    switch (result) {
        case VALIDATE_OK:           return "OK";
        case VALIDATE_FAIL_FREQ:    return "FAIL:FREQ_MISMATCH";
        case VALIDATE_FAIL_DUTY:    return "FAIL:DUTY_MISMATCH";
        case VALIDATE_FAIL_GPIO:    return "FAIL:GPIO_MISMATCH";
        case VALIDATE_FAIL_UART:    return "FAIL:UART_MISMATCH";
        case VALIDATE_FAIL_SPI:     return "FAIL:SPI_MISMATCH";
        case VALIDATE_FAIL_ADC:     return "FAIL:ADC_MISMATCH";
        case VALIDATE_FAIL_TIMEOUT: return "FAIL:TIMEOUT";
        case VALIDATE_FAIL_UNKNOWN: return "FAIL:UNKNOWN_ACTION";
        default:                    return "FAIL:UNKNOWN";
    }
}
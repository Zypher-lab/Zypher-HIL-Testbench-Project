#ifndef PWM_CAPTURE_H
#define PWM_CAPTURE_H

#include <stdint.h>
#include "validator.h"

/* ─── Return codes ───────────────────────────────────────────────────────── */
typedef enum {
    PWM_CAP_OK = 0,
    PWM_CAP_ERR_INVALID_PIN,    /* pin string not recognised                 */
    PWM_CAP_ERR_DEVICE,         /* zephyr device not ready                   */
    PWM_CAP_ERR_TIMEOUT,        /* no signal detected within timeout         */
    PWM_CAP_ERR_IO,             /* capture API returned error                */
} pwm_cap_result_t;

/* ─── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Measure PWM signal on the given pin.
 *         Fills measured->num_values[0] with frequency (Hz)
 *         Fills measured->num_values[1] with duty cycle (%)
 *
 * @param  pin          Pin name string — must match overlay e.g. "PB10"
 * @param  timeout_ms   How long to wait for a signal before giving up
 * @param  measured     Output struct to fill with measured values
 * @return pwm_cap_result_t
 */
pwm_cap_result_t pwm_capture_measure(const char *pin,
                                     uint32_t timeout_ms,
                                     measured_t *measured);

/**
 * @brief  Convert result code to string.
 */
const char *pwm_cap_result_str(pwm_cap_result_t result);

#endif /* PWM_CAPTURE_H */
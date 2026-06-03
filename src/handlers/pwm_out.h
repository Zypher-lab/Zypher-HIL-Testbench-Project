#ifndef PWM_OUT_H
#define PWM_OUT_H

#include <stdint.h>

/* ─── Return codes ───────────────────────────────────────────────────────── */
typedef enum {
    PWM_OUT_OK = 0,
    PWM_OUT_ERR_INVALID_PIN,    /* pin string not recognised                 */
    PWM_OUT_ERR_INVALID_FREQ,   /* frequency is 0 or too high                */
    PWM_OUT_ERR_INVALID_DUTY,   /* duty cycle outside 0-100                  */
    PWM_OUT_ERR_DEVICE,         /* zephyr device not ready                   */
} pwm_out_result_t;

/* ─── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Start PWM output on the given pin.
 *
 * @param  pin        Pin name string — must match overlay e.g. "PA8"
 * @param  freq_hz    Frequency in Hz  (1 – 100000)
 * @param  duty_pct   Duty cycle in %  (0 – 100)
 * @return pwm_out_result_t
 */
pwm_out_result_t pwm_out_start(const char *pin, uint32_t freq_hz, uint32_t duty_pct);

/**
 * @brief  Stop PWM output — sets pin low.
 *
 * @param  pin        Same pin string used in pwm_out_start()
 */
void pwm_out_stop(const char *pin);

/**
 * @brief  Convert result code to string.
 */
const char *pwm_out_result_str(pwm_out_result_t result);

#endif /* PWM_OUT_H */
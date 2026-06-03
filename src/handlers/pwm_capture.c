#include "pwm_capture.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 * get device from the pwm child node — not the timers parent
 *
 * pwm2 label → timers2 → pwm child → TIM2 driver
 * channel 3  → TIM2 CH3 → PB10
 * ───────────────────────────────────────────────────────────── */
static const struct device *pwm2_dev = DEVICE_DT_GET(DT_NODELABEL(pwm2));
#define PWM2_CHANNEL    3U
#define PWM2_FLAGS      0

static const struct device *get_pwm_dev(const char *pin, uint32_t *channel)
{
    if (strcmp(pin, "PB10") == 0) {
        *channel = PWM2_CHANNEL;
        return pwm2_dev;
    }
    return NULL;
}

pwm_cap_result_t pwm_capture_measure(const char *pin,
                                     uint32_t timeout_ms,
                                     measured_t *measured)
{
    uint32_t period_cycles = 0;
    uint32_t pulse_cycles  = 0;
    uint32_t channel;
    int ret;

    const struct device *dev = get_pwm_dev(pin, &channel);
    if (dev == NULL)           return PWM_CAP_ERR_INVALID_PIN;
    if (!device_is_ready(dev)) return PWM_CAP_ERR_DEVICE;

    memset(measured, 0, sizeof(measured_t));

    /* blocking single-shot capture.
     * pwm_capture_cycles() internally configures the callback, enables
     * capture, waits, and disables it again — do NOT call
     * pwm_enable_capture()/pwm_disable_capture() ourselves, or the
     * driver rejects it (no callback configured / already busy). */
    ret = pwm_capture_cycles(dev,
                             channel,
                             PWM_CAPTURE_TYPE_BOTH,
                             &period_cycles,
                             &pulse_cycles,
                             K_MSEC(timeout_ms));

    if (ret == -EAGAIN) return PWM_CAP_ERR_TIMEOUT;
    if (ret != 0)       return PWM_CAP_ERR_IO;
    if (period_cycles == 0) return PWM_CAP_ERR_IO;

    /* prescaler=83 → 1MHz tick
     * freq_hz  = 1,000,000 / period_cycles
     * duty_pct = pulse_cycles * 100 / period_cycles */
    measured->num_values[0]   = 1000000u / period_cycles;
    measured->num_values[1]   = pulse_cycles * 100u / period_cycles;
    measured->num_value_count = 2;

    return PWM_CAP_OK;
}

const char *pwm_cap_result_str(pwm_cap_result_t result)
{
    switch (result) {
        case PWM_CAP_OK:              return "OK";
        case PWM_CAP_ERR_INVALID_PIN: return "FAIL:CAP_INVALID_PIN";
        case PWM_CAP_ERR_DEVICE:      return "FAIL:CAP_DEVICE_ERROR";
        case PWM_CAP_ERR_TIMEOUT:     return "FAIL:CAP_TIMEOUT";
        case PWM_CAP_ERR_IO:          return "FAIL:CAP_IO_ERROR";
        default:                      return "FAIL:UNKNOWN";
    }
}
#include "pwm_out.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 * get device from the pwm child node — not the timers parent
 *
 * pwm1 label → timers1 → pwm child → TIM1 driver
 * channel 1  → TIM1 CH1 → PA8
 *
 * pwm_set() takes the pwm device, not the timers device
 * ───────────────────────────────────────────────────────────── */
static const struct device *pwm1_dev = DEVICE_DT_GET(DT_NODELABEL(pwm1));
#define PWM1_CHANNEL    1U
#define PWM1_FLAGS      0

static const struct device *get_pwm_dev(const char *pin, uint32_t *channel)
{
    if (strcmp(pin, "PA8") == 0) {
        *channel = PWM1_CHANNEL;
        return pwm1_dev;
    }
    return NULL;
}

pwm_out_result_t pwm_out_start(const char *pin, uint32_t freq_hz, uint32_t duty_pct)
{
    uint32_t channel;
    const struct device *dev = get_pwm_dev(pin, &channel);

    if (dev == NULL)                      return PWM_OUT_ERR_INVALID_PIN;
    if (!device_is_ready(dev))            return PWM_OUT_ERR_DEVICE;
    if (freq_hz == 0 || freq_hz > 100000) return PWM_OUT_ERR_INVALID_FREQ;
    if (duty_pct > 100)                   return PWM_OUT_ERR_INVALID_DUTY;

    uint32_t period_ns = 1000000000u / freq_hz;
    uint32_t pulse_ns  = period_ns * duty_pct / 100u;

    int ret = pwm_set(dev, channel, period_ns, pulse_ns, PWM1_FLAGS);
    if (ret != 0) return PWM_OUT_ERR_DEVICE;

    return PWM_OUT_OK;
}

void pwm_out_stop(const char *pin)
{
    uint32_t channel;
    const struct device *dev = get_pwm_dev(pin, &channel);
    if (dev == NULL) return;
    pwm_set(dev, channel, 1000000000u / 1000u, 0, PWM1_FLAGS);
}

const char *pwm_out_result_str(pwm_out_result_t result)
{
    switch (result) {
        case PWM_OUT_OK:               return "OK";
        case PWM_OUT_ERR_INVALID_PIN:  return "FAIL:PWM_INVALID_PIN";
        case PWM_OUT_ERR_INVALID_FREQ: return "FAIL:PWM_INVALID_FREQ";
        case PWM_OUT_ERR_INVALID_DUTY: return "FAIL:PWM_INVALID_DUTY";
        case PWM_OUT_ERR_DEVICE:       return "FAIL:PWM_DEVICE_ERROR";
        default:                       return "FAIL:UNKNOWN";
    }
}
#include "pwm_service.h"
#include "board_map.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <string.h>

#define MCPWM_CH_OUT1 0U
#define MCPWM_CH_IN1 6U

static uint32_t udiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

static bool get_out_channel(const char *name, uint32_t *ch)
{
    if (strcmp(name, "PWM_OUT1") == 0)
    {
        *ch = MCPWM_CH_OUT1;
        return true;
    }
    return false;
}

static bool get_cap_channel(const char *name, uint32_t *ch)
{
    if (strcmp(name, "PWM_IN1") == 0)
    {
        *ch = MCPWM_CH_IN1;
        return true;
    }
    return false;
}

int pwm_service_init(void)
{
    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(mcpwm0));
    if (!device_is_ready(dev))
        return -1;
    return 0;
}

bool pwm_service_write(const char *channel, uint32_t frequency, uint32_t duty_cycle)
{
    uint32_t ch;
    if (!get_out_channel(channel, &ch))
        return false;
    if (frequency == 0 || frequency > 100000)
        return false;
    if (duty_cycle > 100)
        return false;

    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(mcpwm0));
    if (!device_is_ready(dev))
        return false;

    uint32_t period_ns = 1000000000u / frequency;
    uint32_t pulse_ns = period_ns * duty_cycle / 100u;

    printk("period_ns=%u pulse_ns=%u\n",
           period_ns,
           pulse_ns);

    return pwm_set(dev, ch, period_ns, pulse_ns, 0) == 0;
}

static bool capture_once(const struct device *dev,
                         uint32_t ch,
                         uint64_t clk_hz,
                         uint32_t timeout_ms,
                         uint32_t *out_frequency,
                         uint32_t *out_duty_cycle)
{
    uint32_t period_cycles = 0;
    uint32_t pulse_cycles = 0;

    int ret = pwm_capture_cycles(dev,
                                 ch,
                                 PWM_CAPTURE_TYPE_BOTH | PWM_CAPTURE_MODE_SINGLE,
                                 &period_cycles,
                                 &pulse_cycles,
                                 Z_TIMEOUT_MS(timeout_ms));

    if (ret == -EBUSY)
    {
        pwm_disable_capture(dev, ch);
        return false;
    }

    if (ret != 0)
        return false;
    if (period_cycles == 0)
        return false;

    *out_frequency = (uint32_t)(clk_hz / period_cycles);
    *out_duty_cycle = pulse_cycles * 100u / period_cycles;
    printk("ret=%d period=%u pulse=%u\n",
           ret,
           period_cycles,
           pulse_cycles);

    printk("clk=%llu\n", clk_hz);

    printk("freq=%u duty=%u\n",
           *out_frequency,
           *out_duty_cycle);
    pwm_get_cycles_per_sec(dev, ch, &clk_hz);
    printk("capture clock = %llu\n", clk_hz);
    printk("channel=%u\n", ch);

    return true;
}

bool pwm_service_read(const char *channel,
                      uint32_t timeout_ms,
                      uint32_t *out_frequency,
                      uint32_t *out_duty_cycle)
{
    uint32_t ch;
    uint64_t clk_hz = 0;
    uint32_t freq = 0, duty = 0;

    if (!get_cap_channel(channel, &ch))
        return false;

    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(mcpwm0));
    if (!device_is_ready(dev))
        return false;

    pwm_get_cycles_per_sec(dev, ch, &clk_hz);

    /* first capture — flushes stale ISR state, result discarded */
    capture_once(dev, ch, clk_hz, timeout_ms, &freq, &duty);

    *out_frequency = freq;
    *out_duty_cycle = duty;

    return true;
}

bool pwm_service_read_with_tolerance(const char *channel,
                                     uint32_t expected_frequency,
                                     uint32_t expected_duty_cycle,
                                     uint32_t freq_tol_pct,
                                     uint32_t duty_tol_pp,
                                     uint32_t timeout_ms,
                                     uint32_t *out_frequency,
                                     uint32_t *out_duty_cycle)
{
    if (!pwm_service_read(channel, timeout_ms, out_frequency, out_duty_cycle))
    {
        return false;
    }

    if (udiff(*out_frequency, expected_frequency) * 100u > expected_frequency * freq_tol_pct)
    {
        return false;
    }

    if (udiff(*out_duty_cycle, expected_duty_cycle) > duty_tol_pp)
    {
        return false;
    }

    return true;
}
#include "fan_pwm.h"
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>

#define ADC_NODE DT_NODELABEL(adc1)
#define PWM_NODE DT_ALIAS(pwm_fan)
#define PWM_FREQ_HZ 1000u
#define PWM_PERIOD_NS (1000000000u / PWM_FREQ_HZ)
#define ADC_MAX 4095u

static const struct device *adc_dev;
static const struct device *pwm_dev;

static struct adc_channel_cfg ch_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 0,
};

int fan_pwm_init(void)
{
    adc_dev = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc_dev))
        return -1;

    pwm_dev = DEVICE_DT_GET(PWM_NODE);
    if (!device_is_ready(pwm_dev))
        return -1;

    adc_channel_setup(adc_dev, &ch_cfg);
    return 0;
}

void fan_pwm_update(void)
{
    static int count = 0;

    int16_t buf;
    struct adc_sequence seq = {
        .channels = BIT(0),
        .buffer = &buf,
        .buffer_size = sizeof(buf),
        .resolution = 12,
    };

    if (adc_read(adc_dev, &seq) != 0)
        return;

    uint32_t duty_pct = (uint32_t)buf * 100u / ADC_MAX;
    uint32_t pulse_ns = PWM_PERIOD_NS * duty_pct / 100u;
    pwm_set(pwm_dev, 1, PWM_PERIOD_NS, pulse_ns, 0);

    if (++count >= 50)
    {
        count = 0;
        printk("ADC: %d duty: %d%%\n", buf, duty_pct);
    }
}

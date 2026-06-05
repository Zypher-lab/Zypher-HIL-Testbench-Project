#include "adc_app.h"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/*
 * ADC DUT behavior:
 *
 * PA0 = ADC_IN / ADC1 channel 0
 * PB5 = WARNING_OUT
 *
 * ADC raw <  1800 -> PB5 LOW
 * ADC raw >= 1800 -> PB5 HIGH
 *
 * Notes:
 * - STM32F401 ADC is 12-bit.
 * - Raw range is approximately:
 *   0    ~= 0V
 *   4095 ~= 3.3V
 * - 1500mV threshold approximately:
 *   1500 / 3300 * 4095 ~= 1860
 */

#define ADC_DT_SPEC ADC_DT_SPEC_GET(DT_PATH(zephyr_user))

#define WARNING_GPIO_NODE DT_NODELABEL(gpiob)
#define WARNING_OUT_PIN   5

#define ADC_RAW_THRESHOLD 1800

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC;
static const struct device *warning_gpio = DEVICE_DT_GET(WARNING_GPIO_NODE);

static int16_t sample_buffer;

static struct adc_sequence sequence = {
    .buffer = &sample_buffer,
    .buffer_size = sizeof(sample_buffer),
};

int adc_app_init(void)
{
    int ret;

    if (!device_is_ready(warning_gpio)) {
        printk("ERROR: WARNING GPIO device is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure(warning_gpio, WARNING_OUT_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Failed to configure PB5 WARNING_OUT, ret=%d\n", ret);
        return ret;
    }

    if (!adc_is_ready_dt(&adc_channel)) {
        printk("ERROR: ADC device is not ready\n");
        return -1;
    }

    ret = adc_channel_setup_dt(&adc_channel);
    if (ret < 0) {
        printk("ERROR: ADC channel setup failed, ret=%d\n", ret);
        return ret;
    }

    ret = adc_sequence_init_dt(&adc_channel, &sequence);
    if (ret < 0) {
        printk("ERROR: ADC sequence init failed, ret=%d\n", ret);
        return ret;
    }

    printk("ADC app initialized: PA0 ADC_IN -> PB5 WARNING_OUT, raw_threshold=%d\n",
           ADC_RAW_THRESHOLD);

    return 0;
}

void adc_app_update(void)
{
    int ret;

    ret = adc_read_dt(&adc_channel, &sequence);
    if (ret < 0) {
        printk("ERROR: ADC read failed, ret=%d\n", ret);
        return;
    }

    if (sample_buffer >= ADC_RAW_THRESHOLD) {
        gpio_pin_set_raw(warning_gpio, WARNING_OUT_PIN, 1);
    } else {
        gpio_pin_set_raw(warning_gpio, WARNING_OUT_PIN, 0);
    }
}
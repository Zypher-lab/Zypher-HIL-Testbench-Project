#include "switch_led.h"
#include <zephyr/drivers/gpio.h>

#define SWITCH_NODE  DT_NODELABEL(gpiob)
#define SWITCH_PIN   0
#define LED_PIN      1

static const struct device *gpio_dev;

int switch_led_init(void)
{
    gpio_dev = DEVICE_DT_GET(SWITCH_NODE);
    if (!device_is_ready(gpio_dev)) return -1;

    gpio_pin_configure(gpio_dev, SWITCH_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    gpio_pin_configure(gpio_dev, LED_PIN,    GPIO_OUTPUT_INACTIVE);

    return 0;
}

void switch_led_update(void)
{
    int val = gpio_pin_get_raw(gpio_dev, SWITCH_PIN);
    gpio_pin_set_raw(gpio_dev, LED_PIN, val);
}

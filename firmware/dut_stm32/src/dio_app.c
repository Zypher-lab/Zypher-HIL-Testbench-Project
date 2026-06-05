#include "dio_app.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define DIO_GPIO_NODE DT_NODELABEL(gpiob)

#define BUTTON_IN_PIN 0
#define LED_OUT_PIN   1

static const struct device *gpio_b = DEVICE_DT_GET(DIO_GPIO_NODE);

int dio_app_init(void)
{
    int ret;

    if (!device_is_ready(gpio_b)) {
        printk("ERROR: GPIOB device is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure(gpio_b, BUTTON_IN_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0) {
        printk("ERROR: Failed to configure PB0 BUTTON_IN\n");
        return ret;
    }

    ret = gpio_pin_configure(gpio_b, LED_OUT_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Failed to configure PB1 LED_OUT\n");
        return ret;
    }

    printk("DIO app initialized: PB0 -> PB1\n");
    return 0;
}

void dio_app_update(void)
{
    int button_state = gpio_pin_get_raw(gpio_b, BUTTON_IN_PIN);

    if (button_state < 0) {
        printk("ERROR: Failed to read PB0\n");
        return;
    }

    gpio_pin_set_raw(gpio_b, LED_OUT_PIN, button_state ? 1 : 0);
}

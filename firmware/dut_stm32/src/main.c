#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/*
 * STM32 DUT DIO Application
 *
 * Behavior:
 * - BUTTON_IN = PB0
 * - LED_OUT   = PB1
 *
 * If PB0 is HIGH -> PB1 HIGH
 * If PB0 is LOW  -> PB1 LOW
 *
 * Testbench meaning:
 * - ESP32 simulates button signal on PB0
 * - ESP32 monitors DUT LED output from PB1
 */

#define DUT_GPIO_NODE DT_NODELABEL(gpiob)

#define BUTTON_IN_PIN 0
#define LED_OUT_PIN 1

static const struct device *gpio_b = DEVICE_DT_GET(DUT_GPIO_NODE);

int main()
{
    if (!device_is_ready(gpio_b))
    {
        printk("ERROR: GPIOB device is not ready\n");
        return 0;
    }
    int ret = gpio_pin_configure(gpio_b,BUTTON_IN_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    ret = gpio_pin_configure(gpio_b,LED_OUT_PIN, GPIO_OUTPUT_INACTIVE);
    while (1)
    {
        int button_state = gpio_pin_get_raw(gpio_b,BUTTON_IN_PIN);
        gpio_pin_set_raw(gpio_b,LED_OUT_PIN,button_state ? 1 : 0 );
        k_msleep(10);
        
    }
    
}

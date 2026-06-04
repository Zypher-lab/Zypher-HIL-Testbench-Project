#include "gpio_service.h"
#include "board_map.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stddef.h>
#define ESP32_GPIO_NODE DT_NODELABEL(gpio0)

static const struct device *gpio0_dev = DEVICE_DT_GET(ESP32_GPIO_NODE);

int gpio_service_init(void)
{
    if (!device_is_ready(gpio0_dev))
    {
        return -1;
    }

    const board_gpio_channel_t *channels = board_map_get_gpio_channels();
    size_t channel_count = board_map_get_gpio_channel_count();

    for (size_t i = 0; i < channel_count; i++)
    {
        int ret;

        if (channels[i].direction == BOARD_GPIO_OUTPUT)
        {
            ret = gpio_pin_configure(gpio0_dev,
                                     channels[i].pin,
                                     GPIO_OUTPUT_INACTIVE);
        }
        else
        {
            ret = gpio_pin_configure(gpio0_dev,
                                     channels[i].pin,
                                     GPIO_INPUT | GPIO_PULL_DOWN);
        }

        if (ret < 0)
        {
            return -1;
        }
    }

    return 0;
}

bool gpio_service_write(const char *channel_name, int value)
{
    board_gpio_channel_t channel;

    if (!board_map_get_gpio_channel(channel_name, &channel))
    {
        return false;
    }

    if (channel.direction != BOARD_GPIO_OUTPUT)
    {
        return false;
    }

    gpio_pin_set_raw(gpio0_dev, channel.pin, value ? 1 : 0);

    return true;
}

bool gpio_service_read(const char *channel_name, int *out_value)
{
    board_gpio_channel_t channel;

    if (!board_map_get_gpio_channel(channel_name, &channel))
    {
        return false;
    }

    int value = gpio_pin_get_raw(gpio0_dev, channel.pin);

    if (value < 0)
    {
        return false;
    }

    *out_value = value ? 1 : 0;

    return true;
}

bool gpio_service_expect(const char *channel_name,
                         int expected,
                         int timeout_ms,
                         int *out_actual)
{
    int elapsed_ms = 0;
    int actual = 0;

    while (elapsed_ms <= timeout_ms)
    {
        if (!gpio_service_read(channel_name, &actual))
        {
            return false;
        }

        *out_actual = actual;

        if (actual == (expected ? 1 : 0))
        {
            return true;
        }

        k_msleep(5);
        elapsed_ms += 5;
    }

    return false;
}

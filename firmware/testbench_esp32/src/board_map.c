#include "board_map.h"

#include <string.h>

/*
 * ESP32 Testbench GPIO Resource Map
 *
 * DIO_OUT1 -> GPIO32 -> STM32 PB0
 * DIO_IN1  -> GPIO33 <- STM32 PB1
 */

static const board_gpio_channel_t gpio_channels[] = {
    {
        .name = "DIO_OUT1",
        .pin = 27,
        .direction = BOARD_GPIO_OUTPUT,
    },
    {
        .name = "DIO_IN1",
        .pin = 14,
        .direction = BOARD_GPIO_INPUT,
    },
};

bool board_map_get_gpio_channel(const char *name, board_gpio_channel_t *out_channel)
{
    for (int i = 0; i < (int)(sizeof(gpio_channels) / sizeof(gpio_channels[0])); i++) {
        if (strcmp(name, gpio_channels[i].name) == 0) {
            *out_channel = gpio_channels[i];
            return true;
        }
    }

    return false;
}
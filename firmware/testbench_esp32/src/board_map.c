#include "board_map.h"
#include <string.h>

static const board_gpio_channel_t gpio_channels[] = {
    { .name = "DIO_OUT1", .pin = 27, .direction = BOARD_GPIO_OUTPUT },
    { .name = "DIO_OUT2", .pin = 13, .direction = BOARD_GPIO_OUTPUT },
    { .name = "DIO_IN1",  .pin = 14, .direction = BOARD_GPIO_INPUT  },
    { .name = "DIO_IN2",  .pin = 22, .direction = BOARD_GPIO_INPUT  },
};

static const board_pwm_channel_t pwm_channels[] = {
    { .name = "PWM_OUT1", .pin = 32, .direction = BOARD_PWM_OUTPUT },
    { .name = "PWM_IN1",  .pin = 4,  .direction = BOARD_PWM_INPUT  },
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

const board_gpio_channel_t *board_map_get_gpio_channels(void) { return gpio_channels; }
size_t board_map_get_gpio_channel_count(void) { return sizeof(gpio_channels) / sizeof(gpio_channels[0]); }

bool board_map_get_pwm_channel(const char *name, board_pwm_channel_t *out_channel)
{
    for (int i = 0; i < (int)(sizeof(pwm_channels) / sizeof(pwm_channels[0])); i++) {
        if (strcmp(name, pwm_channels[i].name) == 0) {
            *out_channel = pwm_channels[i];
            return true;
        }
    }
    return false;
}

const board_pwm_channel_t *board_map_get_pwm_channels(void) { return pwm_channels; }
size_t board_map_get_pwm_channel_count(void) { return sizeof(pwm_channels) / sizeof(pwm_channels[0]); }
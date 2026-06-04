#ifndef BOARD_MAP_H
#define BOARD_MAP_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    BOARD_GPIO_INPUT = 0,
    BOARD_GPIO_OUTPUT,
} board_gpio_direction_t;

typedef struct {
    const char *name;
    int pin;
    board_gpio_direction_t direction;
} board_gpio_channel_t;

bool board_map_get_gpio_channel(const char *name, board_gpio_channel_t *out_channel);

const board_gpio_channel_t *board_map_get_gpio_channels(void);
size_t board_map_get_gpio_channel_count(void);

#endif
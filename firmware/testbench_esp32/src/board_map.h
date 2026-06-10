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

typedef enum {
    BOARD_PWM_OUTPUT = 0,
    BOARD_PWM_INPUT,
} board_pwm_direction_t;
 
typedef struct {
    const char           *name;
    int                   pin;
    board_pwm_direction_t direction;
} board_pwm_channel_t;

/* SPI is a single fixed channel — no mapping needed */
#define BOARD_SPI_CLK_PIN    18
#define BOARD_SPI_MOSI_PIN   23
#define BOARD_SPI_MISO_PIN   19
#define BOARD_SPI_CS_PIN      5



bool board_map_get_gpio_channel(const char *name, board_gpio_channel_t *out_channel);
const board_gpio_channel_t *board_map_get_gpio_channels(void);
size_t board_map_get_gpio_channel_count(void);



bool board_map_get_pwm_channel(const char *name, board_pwm_channel_t *out_channel);
const board_pwm_channel_t *board_map_get_pwm_channels(void);
size_t board_map_get_pwm_channel_count(void);


#endif
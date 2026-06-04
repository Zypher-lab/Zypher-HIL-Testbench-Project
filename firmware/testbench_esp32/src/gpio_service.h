#ifndef GPIO_SERVICE_H
#define GPIO_SERVICE_H

#include <stdbool.h>

int gpio_service_init(void);

bool gpio_service_write(const char *channel, int value);
bool gpio_service_read(const char *channel, int *out_value);
bool gpio_service_expect(const char *channel,
                         int expected,
                         int timeout_ms,
                         int *out_actual);

#endif
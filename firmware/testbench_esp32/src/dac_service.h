#ifndef DAC_SERVICE_H
#define DAC_SERVICE_H

#include <stdbool.h>

int dac_service_init(void);
bool dac_service_write_mv(const char *channel_name, int mv);

#endif
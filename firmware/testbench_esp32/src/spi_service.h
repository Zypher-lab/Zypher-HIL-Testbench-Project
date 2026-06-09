#ifndef SPI_SERVICE_H
#define SPI_SERVICE_H

#include <stdbool.h>
#include <stddef.h>

int  spi_service_init(void);

bool spi_service_send_expect(const char *tx,
                             const char *expect,
                             char       *out_rx,
                             size_t      out_rx_size);

#endif
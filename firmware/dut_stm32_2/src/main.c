#include <zephyr/kernel.h>
#include "spi_slave.h"

int main(void)
{
    printk("DUT2 SPI Slave starting\n");
    if (spi_slave_init() != 0) return 0;
    spi_slave_run();
    return 0;
}

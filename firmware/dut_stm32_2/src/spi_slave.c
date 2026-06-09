#include "spi_slave.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <string.h>

static const struct device *spi_dev;

static const struct spi_config slave_cfg = {
    .frequency = 100000,
    .operation = SPI_OP_MODE_SLAVE |
                 SPI_TRANSFER_MSB  |
                 SPI_WORD_SET(8)   |
                 SPI_LINES_SINGLE,
    .slave = 0,
};

int spi_slave_init(void)
{
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI slave not ready\n");
        return -1;
    }
    printk("SPI slave ready\n");
    return 0;
}

void spi_slave_run(void)
{
    static const char tx_data[] = "PONG";
    static char       rx_data[8];

    while (1) {
        memset(rx_data, 0, sizeof(rx_data));

        struct spi_buf tx_buf = { .buf = (void *)tx_data, .len = 4 };
        struct spi_buf rx_buf = { .buf = rx_data,         .len = 4 };

        struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
        struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

        int ret = spi_transceive(spi_dev, &slave_cfg, &tx_set, &rx_set);
        if (ret == 0) {
            printk("RX: %s TX: PONG\n", rx_data);
        } else {
            printk("SPI error: %d\n", ret);
        }
    }
}

#include "spi_cmd.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#define SPI_NODE    DT_NODELABEL(spi1)
#define LED_PIN     5
#define GPIOB_NODE  DT_NODELABEL(gpiob)

static const struct device *spi_dev;
static const struct device *gpiob_dev;

static const struct spi_config slave_cfg = {
    .frequency = 500000,
    .operation = SPI_OP_MODE_SLAVE |
                 SPI_TRANSFER_MSB  |
                 SPI_WORD_SET(8)   |
                 SPI_LINES_SINGLE,
    .slave = 0,
};

int spi_cmd_init(void)
{
    spi_dev = DEVICE_DT_GET(SPI_NODE);
    if (!device_is_ready(spi_dev)) {
        printk("SPI1 slave not ready\n");
        return -1;
    }

    gpiob_dev = DEVICE_DT_GET(GPIOB_NODE);
    if (!device_is_ready(gpiob_dev)) return -1;

    gpio_pin_configure(gpiob_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);

    printk("SPI1 slave ready\n");
    return 0;
}

void spi_cmd_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    uint8_t rx_buf[8];
    uint8_t tx_buf[8] = {0};

    while (1) {
        memset(rx_buf, 0, sizeof(rx_buf));

        struct spi_buf rx_bufs[] = {{ .buf = rx_buf, .len = 1 }};
        struct spi_buf tx_bufs[] = {{ .buf = tx_buf, .len = 1 }};

        struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = 1 };
        struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 1 };

        int ret = spi_transceive(spi_dev, &slave_cfg, &tx_set, &rx_set);

        printk("SPI ret=%d rx=0x%02x\n", ret, rx_buf[0]);
        if (ret >= 0) {
            uint8_t cmd = rx_buf[0];
            printk("SPI RX: 0x%02x\n", cmd);

            if (cmd == 0x01) {
                gpio_pin_set_raw(gpiob_dev, LED_PIN, 1);
                printk("SPI CMD: LED_ON\n");
            } else if (cmd == 0x00) {
                gpio_pin_set_raw(gpiob_dev, LED_PIN, 0);
                printk("SPI CMD: LED_OFF\n");
            }
        } else {
            printk("SPI error: %d\n", ret);
        }
    }
}

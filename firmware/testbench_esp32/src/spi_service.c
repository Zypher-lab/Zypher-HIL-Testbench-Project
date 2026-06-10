#include "spi_service.h"
#include "board_map.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <string.h>

#define SPI_NODE    DT_NODELABEL(spi2)
#define SPI_DEV     DT_CHILD(DT_NODELABEL(spi2), spi_dev0)

static const struct spi_dt_spec spi_dev =
    SPI_DT_SPEC_GET(DT_NODELABEL(spi_dev0),
                    SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8),
                    0);

int spi_service_init(void)
{
    if (!spi_is_ready_dt(&spi_dev)) return -1;
    return 0;
}

bool spi_service_send_expect(const char *tx,
                             const char *expect,
                             char       *out_rx,
                             size_t      out_rx_size)
{
    size_t  len    = strlen(tx);
    uint8_t rx_buf[64] = {0};

    if (len == 0 || len > sizeof(rx_buf)) return false;

    struct spi_buf tx_bufs[] = {{ .buf = (void *)tx, .len = len }};
    struct spi_buf rx_bufs[] = {{ .buf = rx_buf,     .len = len }};

    struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = 1 };

    int ret = spi_transceive_dt(&spi_dev, &tx_set, &rx_set);
    if (ret != 0) return false;

    size_t copy_len = (len < out_rx_size - 1) ? len : out_rx_size - 1;
    memcpy(out_rx, rx_buf, copy_len);
    out_rx[copy_len] = '\0';

    return strncmp(out_rx, expect, strlen(expect)) == 0;
}
bool spi_service_write(const char *tx)
{
    size_t  len    = strlen(tx);
    uint8_t rx_buf[64] = {0};

    if (len == 0 || len > sizeof(rx_buf)) return false;

    struct spi_buf tx_bufs[] = {{ .buf = (void *)tx, .len = len }};
    struct spi_buf rx_bufs[] = {{ .buf = rx_buf,     .len = len }};

    struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = 1 };

    return spi_transceive_dt(&spi_dev, &tx_set, &rx_set) == 0;
}

bool spi_service_write_byte(uint8_t byte)
{
    uint8_t tx_buf[1] = { byte };
    uint8_t rx_buf[1] = { 0 };

    struct spi_buf tx_bufs[] = {{ .buf = tx_buf, .len = 1 }};
    struct spi_buf rx_bufs[] = {{ .buf = rx_buf, .len = 1 }};

    struct spi_buf_set tx_set = { .buffers = tx_bufs, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = rx_bufs, .count = 1 };

    return spi_transceive_dt(&spi_dev, &tx_set, &rx_set) == 0;
}

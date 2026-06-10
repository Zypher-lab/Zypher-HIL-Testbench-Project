#include <zephyr/kernel.h>
#include <zephyr/kvss/nvs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/uart.h>

#include "uart_transport.h"
#include "wifi_transport.h"
#include "ztb_protocol.h"
#include "test_executor.h"
#include "gpio_service.h"
#include "dac_service.h"
#include "uart_service.h"
#include "pwm_service.h"
#include "spi_service.h"

#define LINE_BUF_SIZE     128
#define RESPONSE_BUF_SIZE 160

/* ── NVS ──────────────────────────────────────────────────────────────────── */
#define NVS_PARTITION        storage_partition
#define NVS_TRANSPORT_KEY    1

#define TRANSPORT_UART  1
#define TRANSPORT_WIFI  2
#define TRANSPORT_NONE  0xFF

static struct nvs_fs nvs;

static uint8_t nvs_read_transport(void)
{
    const struct flash_area *fa;
    if (flash_area_open(FIXED_PARTITION_ID(NVS_PARTITION), &fa) != 0)
        return TRANSPORT_NONE;
    flash_area_close(fa);

    nvs.flash_device = FIXED_PARTITION_DEVICE(NVS_PARTITION);
    nvs.offset       = FIXED_PARTITION_OFFSET(NVS_PARTITION);
    nvs.sector_size  = 4096;
    nvs.sector_count = 2;

    if (nvs_mount(&nvs) != 0) return TRANSPORT_NONE;

    uint8_t val = TRANSPORT_NONE;
    nvs_read(&nvs, NVS_TRANSPORT_KEY, &val, sizeof(val));
    return val;
}

static void nvs_write_transport(uint8_t transport)
{
    nvs_write(&nvs, NVS_TRANSPORT_KEY, &transport, sizeof(transport));
}

/* ── UART menu ────────────────────────────────────────────────────────────── */
static uint8_t uart_menu(void)
{
    /* always init UART first for the menu */
    uart_transport_init();

    uart_transport_send_line("ZTB|status=BOOT\r\n");
    uart_transport_send_line("Select transport:\r\n");
    uart_transport_send_line("  1 = UART\r\n");
    uart_transport_send_line("  2 = WIFI\r\n");
    uart_transport_send_line("Waiting 5 seconds...\r\n");

    /* wait up to 5 seconds for a single character */
    const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    uint32_t deadline = k_uptime_get_32() + 5000;

    while (k_uptime_get_32() < deadline) {
        unsigned char c;
        if (uart_poll_in(uart_dev, &c) == 0) {
            if (c == '1') return TRANSPORT_UART;
            if (c == '2') return TRANSPORT_WIFI;
        }
        k_msleep(10);
    }

    /* timeout — default to UART */
    uart_transport_send_line("Timeout — defaulting to UART\r\n");
    return TRANSPORT_UART;
}

/* ── Transport function pointers ──────────────────────────────────────────── */
static void (*transport_send)(const char *line);
static void (*transport_read)(char *buf, size_t size);

/* ── Services init ────────────────────────────────────────────────────────── */
static int services_init(void)
{
    if (gpio_service_init() != 0) {
        transport_send("ZTB|status=FAIL|err=GPIO_INIT_FAILED\r\n");
        return -1;
    }
    if (dac_service_init() != 0) {
        transport_send("ZTB|status=FAIL|err=DAC_INIT_FAILED\r\n");
        return -1;
    }
    if (uart_service_init() != 0) {
        transport_send("ZTB|status=FAIL|err=UART_INIT_FAILED\r\n");
        return -1;
    }
    if (pwm_service_init() != 0) {
        transport_send("ZTB|status=FAIL|err=PWM_INIT_FAILED\r\n");
        return -1;
    }
    if (spi_service_init() != 0) {
        transport_send("ZTB|status=FAIL|err=SPI_INIT_FAILED\r\n");
        return -1;
    }
    return 0;
}

/* ── Main ─────────────────────────────────────────────────────────────────── */
int main(void)
{
    char line[LINE_BUF_SIZE];
    char response_line[RESPONSE_BUF_SIZE];
    ztb_command_t  command;
    ztb_response_t response;

    /* read saved transport */
    uint8_t saved = nvs_read_transport();
    uint8_t chosen;

    if (saved == TRANSPORT_UART || saved == TRANSPORT_WIFI) {
        /* use saved transport — no menu */
        chosen = saved;
        /* still init UART briefly to print boot message */
        uart_transport_init();
        if (chosen == TRANSPORT_UART) {
            uart_transport_send_line("ZTB|transport=UART\r\n");
        } else {
            uart_transport_send_line("ZTB|transport=WIFI — connecting...\r\n");
        }
    } else {
        /* first boot or no saved value — show menu */
        chosen = uart_menu();
        nvs_write_transport(chosen);
    }

    /* init chosen transport */
    if (chosen == TRANSPORT_WIFI) {
        if (wifi_transport_init() != 0) {
            uart_transport_send_line("ZTB|status=FAIL|err=WIFI_INIT_FAILED\r\n");
            return 0;
        }
        transport_send = wifi_transport_send_line;
        transport_read = wifi_transport_read_line;
    } else {
        transport_send = uart_transport_send_line;
        transport_read = uart_transport_read_line;
    }

    /* init all peripheral services */
    if (services_init() != 0) return 0;

    transport_send("ZTB|status=READY\r\n");

    /* main ZTB loop */
    while (1) {
        transport_read(line, sizeof(line));

        if (!ztb_parse_frame(line, &command)) {
            ztb_make_fail_response(&response, -1, "PARSE_ERROR");
        } else {
            /* handle TRANSPORT_SWITCH inline */
            if (command.cmd == ZTB_CMD_TRANSPORT_SWITCH) {
                uint8_t new_transport = TRANSPORT_NONE;
                if (strcmp(command.ch, "UART") == 0) new_transport = TRANSPORT_UART;
                if (strcmp(command.ch, "WIFI") == 0) new_transport = TRANSPORT_WIFI;

                if (new_transport == TRANSPORT_NONE) {
                    ztb_make_fail_response(&response, command.seq, "INVALID_TRANSPORT");
                } else {
                    nvs_write_transport(new_transport);
                    ztb_make_ok_response(&response, command.seq);
                    ztb_format_response_with_cmd(&response, &command,
                                                 response_line, sizeof(response_line));
                    transport_send(response_line);
                    k_msleep(100);
                    sys_reboot(SYS_REBOOT_COLD);
                }
            } else {
                test_executor_execute(&command, &response);
            }
        }

        ztb_format_response_with_cmd(&response, &command,
                                     response_line, sizeof(response_line));
        transport_send(response_line);
    }

    return 0;
}
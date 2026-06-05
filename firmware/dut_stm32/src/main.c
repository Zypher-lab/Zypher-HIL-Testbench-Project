#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "dio_app.h"
#include "adc_app.h"

int main(void)
{
    printk("STM32 integrated DUT started\n");

    if (dio_app_init() != 0) {
        printk("ERROR: DIO app init failed\n");
        return 0;
    }

    if (adc_app_init() != 0) {
        printk("ERROR: ADC app init failed\n");
        return 0;
    }

    while (1) {
        dio_app_update();
        adc_app_update();

        k_msleep(10);
    }

    return 0;
}
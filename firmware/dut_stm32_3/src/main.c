#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "switch_led.h"
#include "fan_pwm.h"
#include "uart_echo.h"
#include "pwm_match.h"

K_SEM_DEFINE(pwm_match_ready, 0, 1);

K_THREAD_DEFINE(pwm_match_tid, 2048,
                pwm_match_thread, NULL, NULL, NULL,
                5, 0, 0);

int main(void)
{
    printk("DUT3 starting\n");

    if (switch_led_init() != 0) {
        printk("switch_led init failed\n");
        return 0;
    }

    if (fan_pwm_init() != 0) {
        printk("fan_pwm init failed\n");
        return 0;
    }

    if (uart_echo_init() != 0) {
        printk("uart_echo init failed\n");
        return 0;
    }

    if (pwm_match_init() != 0) {
        printk("pwm_match init failed\n");
        return 0;
    }

    k_sem_give(&pwm_match_ready);

    printk("DUT3 ready\n");

    while (1) {
        switch_led_update();
        fan_pwm_update();
        k_msleep(10);
    }

    return 0;
}

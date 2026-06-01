
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define USER_NODE DT_PATH(zephyr_user)

static const struct gpio_dt_spec tb_out =
    GPIO_DT_SPEC_GET(USER_NODE, tb_out_gpios);

static const struct gpio_dt_spec dut_in =
    GPIO_DT_SPEC_GET(USER_NODE, dut_in_gpios);

int main(void)
{
    int ret;
    int value;

    if (!gpio_is_ready_dt(&tb_out)) {
        printk("TB output GPIO is not ready\n");
        return 0;
    }

    if (!gpio_is_ready_dt(&dut_in)) {
        printk("DUT input GPIO is not ready\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&tb_out, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure TB output pin\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&dut_in, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure DUT input pin\n");
        return 0;
    }

    gpio_pin_set_dt(&tb_out, 1);

    k_msleep(10);

    value = gpio_pin_get_dt(&dut_in);

    if (value == 1) {
        printk("TEST PASS: PA0 HIGH received on PA1\n");
    } else {
        printk("TEST FAIL: PA1 did not read HIGH\n");
    }

    return 0;
}
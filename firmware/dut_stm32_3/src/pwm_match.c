#include "pwm_match.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

/*
 * Reads PWM signal on PB4 via GPIO interrupt (3-edge capture)
 * Reproduces same freq/duty on PB10 via TIM2 CH3
 */

#define CAPTURE_PIN     4
#define GPIOB_NODE      DT_NODELABEL(gpiob)
#define PWM_MATCH_NODE  DT_ALIAS(pwm_match)
#define EDGES_NEEDED    3

static const struct device *gpio_dev;
static const struct device *pwm_dev;
static struct gpio_callback  cap_cb;

static volatile uint32_t edge_times[EDGES_NEEDED];
static volatile int      edge_levels[EDGES_NEEDED];
static volatile int      edge_count;
static struct k_sem      cap_sem;
static volatile bool     cap_active;

static void capture_isr(const struct device *dev,
                        struct gpio_callback *cb,
                        uint32_t pins)
{
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    if (!cap_active) return;

    int idx = edge_count;
    if (idx < EDGES_NEEDED) {
        edge_times[idx]  = k_cycle_get_32();
        edge_levels[idx] = gpio_pin_get_raw(dev, CAPTURE_PIN);
        edge_count = idx + 1;
    }

    if (edge_count == EDGES_NEEDED) {
        cap_active = false;
        k_sem_give(&cap_sem);
    }
}

int pwm_match_init(void)
{
    gpio_dev = DEVICE_DT_GET(GPIOB_NODE);
    if (!device_is_ready(gpio_dev)) return -1;

    pwm_dev = DEVICE_DT_GET(PWM_MATCH_NODE);
    if (!device_is_ready(pwm_dev)) return -1;

    gpio_pin_configure(gpio_dev, CAPTURE_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    k_sem_init(&cap_sem, 0, 1);
    cap_active  = false;
    edge_count  = 0;

    gpio_init_callback(&cap_cb, capture_isr, BIT(CAPTURE_PIN));
    gpio_add_callback(gpio_dev, &cap_cb);

    return 0;
}

void pwm_match_update(void)
{
    edge_count = 0;
    cap_active = false;
    k_sem_reset(&cap_sem);

    gpio_pin_interrupt_configure(gpio_dev, CAPTURE_PIN, GPIO_INT_EDGE_BOTH);
    cap_active = true;

    /* wait up to 500ms for 3 edges */
    int ret = k_sem_take(&cap_sem, K_MSEC(500));

    gpio_pin_interrupt_configure(gpio_dev, CAPTURE_PIN, GPIO_INT_DISABLE);
    cap_active = false;

    if (ret != 0) { printk("match: timeout\n"); return; }
    if (edge_count < EDGES_NEEDED) return;

    uint32_t cpu_hz    = sys_clock_hw_cycles_per_sec();
    uint32_t period_cy = edge_times[2] - edge_times[0];
    uint32_t pulse_cy;

    if (edge_levels[0] == 1) {
        pulse_cy = edge_times[1] - edge_times[0];
    } else {
        pulse_cy = edge_times[2] - edge_times[1];
    }

    if (period_cy == 0 || pulse_cy == 0 || pulse_cy >= period_cy) { printk("match: bad edge data\n"); return; }

    uint32_t freq_hz   = cpu_hz / period_cy;
    uint32_t duty_pct  = pulse_cy * 100u / period_cy;

    if (freq_hz == 0 || freq_hz > 10000) { printk("match: freq out of range %u\n", freq_hz); return; }

    uint32_t period_ns = 1000000000u / freq_hz;
    uint32_t pulse_ns  = period_ns * duty_pct / 100u;

    printk("match: freq=%u duty=%u\n", freq_hz, duty_pct);
    int r = pwm_set(pwm_dev, 3, period_ns, pulse_ns, 0);
    printk("pwm_set ret=%d\n", r);
}

extern struct k_sem pwm_match_ready;

void pwm_match_thread(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    k_sem_take(&pwm_match_ready, K_FOREVER);
    printk("pwm_match thread started\n");
    while (1) {
        pwm_match_update();
    }
}
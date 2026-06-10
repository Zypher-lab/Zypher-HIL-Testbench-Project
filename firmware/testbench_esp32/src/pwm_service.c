#include "pwm_service.h"
#include "board_map.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

/*
 * PWM output  — LEDC channel 0 — GPIO32 — PWM_OUT1
 * PWM capture — GPIO interrupt — GPIO4  — PWM_IN1 (fan readback)
 * PWM capture — GPIO interrupt — GPIO2  — PWM_IN2 (PWM match readback)
 */

#define LEDC_CH_OUT1        0U
#define PWM_CAPTURE_NODE    DT_NODELABEL(gpio0)
#define EDGES_NEEDED        3

static const struct device *cap_gpio_dev;

/* capture state — one set per channel */
struct cap_state {
    struct gpio_callback  cb;
    volatile uint32_t     edge_times[EDGES_NEEDED];
    volatile int          edge_levels[EDGES_NEEDED];
    volatile int          edge_count;
    struct k_sem          sem;
    volatile bool         active;
    int                   pin;
};

static struct cap_state cap_in1 = { .pin = 4 };
static struct cap_state cap_in2 = { .pin = 2 };

static uint32_t udiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

static void capture_isr(const struct device *dev,
                        struct gpio_callback *cb,
                        uint32_t pins)
{
    struct cap_state *cap = CONTAINER_OF(cb, struct cap_state, cb);
    if (!(pins & BIT(cap->pin))) return;

    if (!cap->active) return;

    int idx = cap->edge_count;
    if (idx < EDGES_NEEDED) {
        cap->edge_times[idx]  = k_cycle_get_32();
        cap->edge_levels[idx] = gpio_pin_get_raw(dev, cap->pin);
        cap->edge_count = idx + 1;
    }

    if (cap->edge_count == EDGES_NEEDED) {
        cap->active = false;
        k_sem_give(&cap->sem);
    }
}

static int init_capture_pin(struct cap_state *cap)
{
    gpio_pin_configure(cap_gpio_dev, cap->pin, GPIO_INPUT | GPIO_PULL_DOWN);
    k_sem_init(&cap->sem, 0, 1);
    cap->active     = false;
    cap->edge_count = 0;

    gpio_init_callback(&cap->cb, capture_isr, BIT(cap->pin));
    return gpio_add_callback(cap_gpio_dev, &cap->cb);
}

int pwm_service_init(void)
{
    const struct device *ledc = DEVICE_DT_GET(DT_NODELABEL(ledc0));
    if (!device_is_ready(ledc)) return -1;

    cap_gpio_dev = DEVICE_DT_GET(PWM_CAPTURE_NODE);
    if (!device_is_ready(cap_gpio_dev)) return -1;

    if (init_capture_pin(&cap_in1) != 0) return -1;
    if (init_capture_pin(&cap_in2) != 0) return -1;

    return 0;
}

bool pwm_service_write(const char *channel, uint32_t frequency, uint32_t duty_cycle)
{
    if (strcmp(channel, "PWM_OUT1") != 0) return false;
    if (frequency == 0 || frequency > 100000) return false;
    if (duty_cycle > 100) return false;

    const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(ledc0));
    if (!device_is_ready(dev)) return false;

    uint32_t period_ns = 1000000000u / frequency;
    uint32_t pulse_ns  = period_ns * duty_cycle / 100u;

    return pwm_set(dev, LEDC_CH_OUT1, period_ns, pulse_ns, 0) == 0;
}

static bool gpio_capture(struct cap_state *cap,
                         uint32_t timeout_ms,
                         uint32_t *out_frequency,
                         uint32_t *out_duty_cycle)
{
    uint32_t cpu_hz = sys_clock_hw_cycles_per_sec();
    uint32_t min_period_cy = cpu_hz / 20000;

    cap->edge_count = 0;
    cap->active     = false;
    k_sem_reset(&cap->sem);

    int ret = gpio_pin_interrupt_configure(cap_gpio_dev, cap->pin, GPIO_INT_EDGE_BOTH);
    if (ret != 0) return false;

    cap->active = true;

    ret = k_sem_take(&cap->sem, K_MSEC(timeout_ms));

    gpio_pin_interrupt_configure(cap_gpio_dev, cap->pin, GPIO_INT_DISABLE);
    cap->active = false;

    if (ret != 0)                      return false;
    if (cap->edge_count < EDGES_NEEDED) return false;

    uint32_t period_cy = cap->edge_times[2] - cap->edge_times[0];
    uint32_t pulse_cy;

    if (cap->edge_levels[0] == 1) {
        pulse_cy = cap->edge_times[1] - cap->edge_times[0];
    } else {
        pulse_cy = cap->edge_times[2] - cap->edge_times[1];
    }

    if (period_cy == 0)            return false;
    if (period_cy < min_period_cy) return false;
    if (pulse_cy  == 0)            return false;
    if (pulse_cy  >= period_cy)    return false;

    *out_frequency  = cpu_hz / period_cy;
    *out_duty_cycle = pulse_cy * 100u / period_cy;

    return true;
}

bool pwm_service_read_with_tolerance(const char *channel,
                                     uint32_t expected_frequency,
                                     uint32_t expected_duty_cycle,
                                     uint32_t freq_tol_pct,
                                     uint32_t duty_tol_pp,
                                     uint32_t timeout_ms,
                                     uint32_t *out_frequency,
                                     uint32_t *out_duty_cycle)
{
    struct cap_state *cap;

    if      (strcmp(channel, "PWM_IN1") == 0) cap = &cap_in1;
    else if (strcmp(channel, "PWM_IN2") == 0) cap = &cap_in2;
    else return false;

    k_msleep(50);

    if (!gpio_capture(cap, timeout_ms, out_frequency, out_duty_cycle)) {
        return false;
    }

    if (udiff(*out_frequency, expected_frequency) * 100u
            > expected_frequency * freq_tol_pct) {
        return false;
    }

    if (udiff(*out_duty_cycle, expected_duty_cycle) > duty_tol_pp) {
        return false;
    }

    return true;
}
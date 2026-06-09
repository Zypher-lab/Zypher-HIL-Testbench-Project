#include "pwm_service.h"
#include "board_map.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

/*
 * PWM output  — LEDC channel 0 — GPIO32 — PWM_OUT1
 *   LEDC handles wide frequency range (1Hz-100kHz) automatically
 *
 * PWM capture — GPIO interrupt — GPIO4 — PWM_IN1
 *   3 edges: rising→falling→rising gives period and pulse
 *   Uses k_cycle_get_32() at 240MHz CPU clock
 */

#define LEDC_CH_OUT1        0U
#define PWM_CAPTURE_GPIO    4
#define PWM_CAPTURE_NODE    DT_NODELABEL(gpio0)
#define EDGES_NEEDED        3

static const struct device *cap_gpio_dev;
static struct gpio_callback  cap_gpio_cb;

static volatile uint32_t edge_times[EDGES_NEEDED];
static volatile int      edge_levels[EDGES_NEEDED];
static volatile int      edge_count;
static struct k_sem      cap_sem;
static volatile bool     cap_active;

static uint32_t udiff(uint32_t a, uint32_t b)
{
    return (a > b) ? (a - b) : (b - a);
}

static void capture_gpio_isr(const struct device *dev,
                              struct gpio_callback *cb,
                              uint32_t pins)
{
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    if (!cap_active) return;

    int idx = edge_count;
    if (idx < EDGES_NEEDED) {
        edge_times[idx]  = k_cycle_get_32();
        edge_levels[idx] = gpio_pin_get_raw(dev, PWM_CAPTURE_GPIO);
        edge_count = idx + 1;
    }

    if (edge_count == EDGES_NEEDED) {
        cap_active = false;
        k_sem_give(&cap_sem);
    }
}

int pwm_service_init(void)
{
    const struct device *ledc = DEVICE_DT_GET(DT_NODELABEL(ledc0));
    if (!device_is_ready(ledc)) return -1;

    cap_gpio_dev = DEVICE_DT_GET(PWM_CAPTURE_NODE);
    if (!device_is_ready(cap_gpio_dev)) return -1;

    int ret = gpio_pin_configure(cap_gpio_dev, PWM_CAPTURE_GPIO,
                                 GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret != 0) return -1;

    k_sem_init(&cap_sem, 0, 1);
    cap_active = false;
    edge_count = 0;

    gpio_init_callback(&cap_gpio_cb, capture_gpio_isr, BIT(PWM_CAPTURE_GPIO));
    ret = gpio_add_callback(cap_gpio_dev, &cap_gpio_cb);
    if (ret != 0) return -1;

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

static bool gpio_capture(uint32_t timeout_ms,
                         uint32_t *out_frequency,
                         uint32_t *out_duty_cycle)
{
    uint32_t cpu_hz = sys_clock_hw_cycles_per_sec();

    /* reject anything faster than 20kHz — noise filter */
    uint32_t min_period_cy = cpu_hz / 20000;

    edge_count = 0;
    cap_active = false;
    k_sem_reset(&cap_sem);

    int ret = gpio_pin_interrupt_configure(cap_gpio_dev,
                                           PWM_CAPTURE_GPIO,
                                           GPIO_INT_EDGE_BOTH);
    if (ret != 0) return false;

    cap_active = true;

    ret = k_sem_take(&cap_sem, K_MSEC(timeout_ms));

    gpio_pin_interrupt_configure(cap_gpio_dev, PWM_CAPTURE_GPIO, GPIO_INT_DISABLE);
    cap_active = false;

    if (ret != 0)                  return false;
    if (edge_count < EDGES_NEEDED) return false;

    uint32_t period_cy = edge_times[2] - edge_times[0];
    uint32_t pulse_cy;

    /* edge_levels[0]=1 means first edge was rising → high time = edge[1]-edge[0]
     * edge_levels[0]=0 means first edge was falling → high time = edge[2]-edge[1] */
    if (edge_levels[0] == 1) {
        pulse_cy = edge_times[1] - edge_times[0];
    } else {
        pulse_cy = edge_times[2] - edge_times[1];
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
    if (strcmp(channel, "PWM_IN1") != 0) return false;

    k_msleep(50);

    if (!gpio_capture(timeout_ms, out_frequency, out_duty_cycle)) {
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
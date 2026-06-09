#ifndef PWM_SERVICE_H
#define PWM_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

int  pwm_service_init(void);

bool pwm_service_write(const char *channel, uint32_t frequency, uint32_t duty_cycle);

bool pwm_service_read(const char *channel,
                      uint32_t timeout_ms,
                      uint32_t *out_frequency,
                      uint32_t *out_duty_cycle);

bool pwm_service_read_with_tolerance(const char *channel,
                        uint32_t expected_frequency,
                        uint32_t expected_duty_cycle,
                        uint32_t freq_tol_pct,
                        uint32_t duty_tol_pp,
                        uint32_t timeout_ms,
                        uint32_t *out_frequency,
                        uint32_t *out_duty_cycle);

#endif
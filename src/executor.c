#include "executor.h"
#include "validator.h"
#include "handlers/pwm_out.h"
#include "handlers/pwm_capture.h"

#include <zephyr/kernel.h>
#include <string.h>

/* settle time after SET before measuring — fixed 50ms */
#define SETTLE_MS   50

/* ─────────────────────────────────────────────────────────────
 * execute one SET operation
 * ───────────────────────────────────────────────────────────── */
static int execute_set(const operation_t *op, char *resp, uint16_t resp_len)
{
    switch (op->action) {

        case ACTION_PWM: {
            /* num_params[0] = frequency (Hz)
             * num_params[1] = duty cycle (%)  */
            pwm_out_result_t res = pwm_out_start(op->pin,
                                                  op->num_params[0],
                                                  op->num_params[1]);
            if (res != PWM_OUT_OK) {
                strncpy(resp, pwm_out_result_str(res), resp_len - 1);
                return -1;
            }
            return 0;
        }

        default:
            strncpy(resp, "FAIL:UNKNOWN_SET_ACTION", resp_len - 1);
            return -1;
    }
}

/* ─────────────────────────────────────────────────────────────
 * execute one ASSERT operation
 * ───────────────────────────────────────────────────────────── */
static int execute_assert(const operation_t *op,
                          uint32_t timeout_ms,
                          char *resp,
                          uint16_t resp_len)
{
    measured_t measured;
    validate_result_t vres;

    switch (op->action) {

        case ACTION_PWM: {
            /* measure PWM on the assert pin */
            pwm_cap_result_t res = pwm_capture_measure(op->pin,
                                                        timeout_ms,
                                                        &measured);
            /* capture failed — timeout or device error */
            if (res != PWM_CAP_OK) {
                strncpy(resp, pwm_cap_result_str(res), resp_len - 1);
                return -1;
            }

            /* validate measured vs expected */
            vres = validate(op, &measured);
            if (vres != VALIDATE_OK) {
                strncpy(resp, validate_result_str(vres), resp_len - 1);
                return -1;
            }

            return 0;
        }

        default:
            strncpy(resp, "FAIL:UNKNOWN_ASSERT_ACTION", resp_len - 1);
            return -1;
    }
}

/* ─────────────────────────────────────────────────────────────
 * main executor
 *
 * phase 1 — SET:    drive all set operations
 * phase 2 — WAIT:   fixed settle time (50ms)
 * phase 3 — ASSERT: measure and validate all assert operations
 *           uses timeout_ms from command as capture window
 * ───────────────────────────────────────────────────────────── */
void executor_run(const parsed_cmd_t *cmd, char *resp, uint16_t resp_len)
{
    memset(resp, 0, resp_len);

    /* ── phase 1: SET ── */
    for (uint8_t i = 0; i < cmd->set_count; i++) {
        if (execute_set(&cmd->set_ops[i], resp, resp_len) != 0) {
            return;
        }
    }

    /* ── phase 2: WAIT ──
     * fixed settle time — gives signal time to stabilise
     * separate from capture timeout                          */
    k_msleep(SETTLE_MS);

    /* ── phase 3: ASSERT ── */
    for (uint8_t i = 0; i < cmd->assert_count; i++) {
        if (execute_assert(&cmd->assert_ops[i],
                           cmd->timeout_ms,
                           resp,
                           resp_len) != 0) {
            return;
        }
    }

    strncpy(resp, "OK", resp_len - 1);
}
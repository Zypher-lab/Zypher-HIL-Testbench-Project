#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "command_parser.h"

/* ─── Return codes ───────────────────────────────────────────────────────── */
typedef enum {
    EXEC_OK = 0,
    EXEC_FAIL_SET,              /* something went wrong during SET phase      */
    EXEC_FAIL_ASSERT,           /* assertion failed — measured != expected    */
    EXEC_FAIL_UNKNOWN_ACTION,   /* action type not supported                  */
    EXEC_FAIL_CAPTURE,          /* capture/measurement failed                 */
} exec_result_t;

/* ─── API ────────────────────────────────────────────────────────────────── */

/**
 * @brief  Execute a fully parsed command.
 *
 *         Phase 1 — SET:    loop through set_ops, drive each one
 *         Phase 2 — WAIT:   wait timeout_ms for DUT to settle
 *         Phase 3 — ASSERT: loop through assert_ops, measure and validate each
 *
 * @param  cmd        Fully parsed command from command_parse()
 * @param  resp       Buffer to write the response string into
 * @param  resp_len   Size of response buffer
 */
void executor_run(const parsed_cmd_t *cmd, char *resp, uint16_t resp_len);

#endif /* EXECUTOR_H */
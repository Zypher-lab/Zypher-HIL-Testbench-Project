#include <zephyr/kernel.h>

#include "serial_handler.h"
#include "command_parser.h"
#include "executor.h"

#define RESP_BUF_SIZE   64
static char resp[RESP_BUF_SIZE];

/* ─────────────────────────────────────────────────────────────
 * main — all blocking work happens here in main loop
 * interrupt only sets a flag in serial_handler
 *
 * this is critical — pwm_capture_cycles blocks for timeout_ms
 * it CANNOT run inside an interrupt callback
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
    /* static: parsed_cmd_t is ~2.5 KB — too large for the main stack
     * (CONFIG_MAIN_STACK_SIZE=2048). Single-threaded loop, so one
     * shared instance is safe and keeps it off the stack. */
    static parsed_cmd_t cmd;
    parse_result_t  pres;
    const char     *frame;

    /* init UART interrupt */
    serial_handler_init();

    while (1) {

        /* wait for a complete frame — sleep 1ms between checks */
        if (!serial_frame_ready()) {
            k_msleep(1);
            continue;
        }

        /* get the frame — clears the ready flag */
        frame = serial_get_frame();

        /* parse */
        pres = command_parse(frame, &cmd);
        if (pres != PARSE_OK) {
            serial_send(parse_result_str(pres));
            continue;
        }

        /* execute — blocking operations happen here safely */
        executor_run(&cmd, resp, RESP_BUF_SIZE);

        /* send response */
        serial_send(resp);
    }

    return 0;
}
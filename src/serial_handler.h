#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_BUF_SIZE     256

/* ─────────────────────────────────────────────────────────────
 * init UART interrupt — call once from main
 * ───────────────────────────────────────────────────────────── */
void serial_handler_init(void);

/* ─────────────────────────────────────────────────────────────
 * check if a complete frame has arrived
 * call from main loop — NOT from interrupt context
 * ───────────────────────────────────────────────────────────── */
bool serial_frame_ready(void);

/* ─────────────────────────────────────────────────────────────
 * get the received frame — call after serial_frame_ready()
 * clears the ready flag so next frame can be received
 * ───────────────────────────────────────────────────────────── */
const char *serial_get_frame(void);

/* ─────────────────────────────────────────────────────────────
 * send a string over UART
 * ───────────────────────────────────────────────────────────── */
void serial_send(const char *msg);

#endif /* SERIAL_HANDLER_H */
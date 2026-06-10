#ifndef WIFI_TRANSPORT_H
#define WIFI_TRANSPORT_H

#include <stddef.h>

int  wifi_transport_init(void);
void wifi_transport_read_line(char *buffer, size_t buffer_size);
void wifi_transport_send_line(const char *line);

#endif

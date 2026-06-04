#ifndef TEST_EXECUTOR_H
#define TEST_EXECUTOR_H

#include "ztb_protocol.h"

void test_executor_execute(const ztb_command_t *command,
                           ztb_response_t *response);

#endif
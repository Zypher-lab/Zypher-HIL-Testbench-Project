#include "test_executor.h"
#include "gpio_service.h"

static void execute_gpio_write(const ztb_command_t *command,
                               ztb_response_t *response)
{
    if (!gpio_service_write(command->ch, command->val)) {
        ztb_make_fail_response(response, command->seq, "GPIO_WRITE_FAILED");
        return;
    }

    ztb_make_ok_response(response, command->seq);
}

static void execute_gpio_read(const ztb_command_t *command,
                              ztb_response_t *response)
{
    int value = 0;

    if (!gpio_service_read(command->ch, &value)) {
        ztb_make_fail_response(response, command->seq, "GPIO_READ_FAILED");
        return;
    }

    ztb_make_gpio_value_response(response, command->seq, value);
}

static void execute_gpio_expect(const ztb_command_t *command,
                                ztb_response_t *response)
{
    int actual = 0;

    bool pass = gpio_service_expect(command->ch,
                                    command->val,
                                    command->timeout_ms,
                                    &actual);

    ztb_make_gpio_expect_response(response,
                                  command->seq,
                                  pass,
                                  command->val,
                                  actual);
}

void test_executor_execute(const ztb_command_t *command,
                           ztb_response_t *response)
{
    switch (command->cmd) {
    case ZTB_CMD_GPIO_WRITE:
        execute_gpio_write(command, response);
        break;

    case ZTB_CMD_GPIO_READ:
        execute_gpio_read(command, response);
        break;

    case ZTB_CMD_GPIO_EXPECT:
        execute_gpio_expect(command, response);
        break;

    default:
        ztb_make_fail_response(response, command->seq, "UNKNOWN_COMMAND");
        break;
    }
}
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp32_generic_testbench)

target_sources(app PRIVATE
    src/main.c
    src/uart_transport.c
    src/ztb_protocol.c
    src/test_executor.c
    src/gpio_service.c
    src/board_map.c
    src/dac_service.c
    src/uart_service.c
    src/pwm_service.c
    src/spi_service.c
)
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dut_stm32_integrated)

target_sources(app PRIVATE
    src/main.c
    src/dio_app.c
    src/adc_app.c
    src/uart_app.c

)

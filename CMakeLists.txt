cmake_minimum_required(VERSION 3.20.0)

# ─── Point to our overlay before Zephyr loads ─────────────────────────────────
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/dts/overlay.dts)

# ─── Zephyr package ───────────────────────────────────────────────────────────
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(testbench)

# ─── Main executable ──────────────────────────────────────────────────────────
target_sources(app PRIVATE src/main.c)

# ─── Source library ───────────────────────────────────────────────────────────
target_sources(app PRIVATE
    src/serial_handler.c
    src/command_parser.c
    src/executor.c
    src/validator.c
    src/handlers/pwm_out.c
    src/handlers/pwm_capture.c
)

# ─── Include paths ────────────────────────────────────────────────────────────
target_include_directories(app PRIVATE
    src
    src/handlers
)
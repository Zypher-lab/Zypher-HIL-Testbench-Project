*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB1
${BAUDRATE}    115200

*** Test Cases ***

# ── SPI_SEND_EXPECT ────────────────────────────────────────────────────────────

# ESP32 sends PING over SPI to STM32 slave.

# STM32 must respond with PONG.

#

# Frame:    ZTB|seq=1|cmd=SPI_SEND_EXPECT|tx=PING|expect=PONG

# Response: ZTB|seq=1|status=OK|rx=PONG

TC_SPI_001_PING_PONG
    [Documentation]    Send PING to STM32 SPI slave, expect PONG back
    SPI Send Expect    1    PING    PONG
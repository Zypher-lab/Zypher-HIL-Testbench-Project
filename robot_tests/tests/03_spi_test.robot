*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***

# ── SPI_SEND_EXPECT ────────────────────────────────────────────────────────────

# ESP32 sends PING over SPI to STM32 slave.

# STM32 must respond with PONG.

#

# Frame:    ZTB|seq=1|cmd=SPI_SEND_EXPECT|tx=PING|expect=PONG

# Response: ZTB|seq=1|status=OK|rx=PONG

TC_1_SPI
    [Documentation]    Send LED_ON over SPI → STM32 sets PB5 HIGH → ESP32 reads DIO_IN2 HIGH
    SPI Write             13    LED_ON
    Sleep                 0.1
    Expect GPIO Value     14    DIO_IN2    1    500

TC_2_SPI
    [Documentation]    Send LED_OFF over SPI → STM32 sets PB5 LOW → ESP32 reads DIO_IN2 LOW
    SPI Write             15    LED_OFF
    Sleep                 0.1
    Expect GPIO Value     16    DIO_IN2    0    500


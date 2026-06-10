*** Settings ***
Library         ../libraries/TestbenchWifi.py
Resource        ../resources/common_keywords_wifi.robot
Suite Setup     Open Testbench    192.168.0.85    5000
Suite Teardown  Close Testbench

*** Variables ***
${HOST}     192.168.0.85
${PORT}     5000

*** Test Cases ***

# ── GPIO ──────────────────────────────────────────────────────
TC_WIFI_GPIO_001_OUT_HIGH
    [Documentation]    GPIO write HIGH over WiFi
    Send GPIO Write      1    DIO_OUT1    1

TC_WIFI_GPIO_002_OUT_LOW
    [Documentation]    GPIO write LOW over WiFi
    Send GPIO Write      2    DIO_OUT1    0

# ── PWM ───────────────────────────────────────────────────────
TC_WIFI_PWM_001_WRITE
    [Documentation]    PWM write over WiFi
    PWM Write    3    PWM_OUT1    100    50

TC_WIFI_PWM_002_READ
    [Documentation]    PWM write and validate over WiFi
    PWM Write                  4    PWM_OUT1    100    50
    PWM Read With Tolerance    5    PWM_IN2     100    50    2000    5    2

TC_WIFI_PWM_003_200HZ
    [Documentation]    PWM 200Hz over WiFi
    PWM Write                  6    PWM_OUT1    200    75
    PWM Read With Tolerance    7    PWM_IN2     200    75    2000    5    2

# ── DAC ───────────────────────────────────────────────────────
TC_WIFI_DAC_001
    [Documentation]    DAC write over WiFi
    Write DAC Voltage    8    DAC_OUT1    1650

TC_WIFI_DAC_002
    [Documentation]    DAC full voltage over WiFi
    Write DAC Voltage    9    DAC_OUT1    3000

# ── UART ──────────────────────────────────────────────────────
TC_WIFI_UART_001_ECHO
    [Documentation]    UART echo over WiFi
    UART Send Expect    10    UART_CH1    PING    PING    1000

TC_WIFI_UART_002_ECHO
    [Documentation]    UART echo HELLO over WiFi
    UART Send Expect    11    UART_CH1    HELLO    HELLO    1000

# ── SPI ───────────────────────────────────────────────────────
TC_WIFI_SPI_001_WRITE
    [Documentation]    SPI write over WiFi
    SPI Write    12    LED_ON

TC_WIFI_SPI_002_WRITE
    [Documentation]    SPI write LED OFF over WiFi
    SPI Write    13    LED_OFF

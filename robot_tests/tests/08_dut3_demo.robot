*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***

# ── Switch → LED ───────────────────────────────────────────────────────────────

# ESP32 drives DIO_OUT1 (GPIO27) → STM32 PB0

# DUT mirrors PB0 to PB1 → ESP32 reads DIO_IN1 (GPIO14)

TC_DUT3_001_SWITCH_ON
    [Documentation]    Switch HIGH — DUT mirrors PB0→PB1 HIGH
    Send GPIO Write      1    DIO_OUT1    1
    Sleep                0.05
    Expect GPIO Value    2    DIO_IN1     1    500

TC_DUT3_002_SWITCH_OFF
    [Documentation]    Switch LOW — DUT mirrors PB0→PB1 LOW
    Send GPIO Write      3    DIO_OUT1    0
    Expect GPIO Value    4    DIO_IN1     0    500


# ── DAC → ADC → Fan PWM ────────────────────────────────────────────────────────

# ESP32 DAC_OUT1 (GPIO25) → STM32 PA0 (ADC)

# DUT ADC reading → sets TIM1 PWM duty on PA8

# ESP32 reads PWM_IN1 (GPIO4 ← PA8)

TC_DUT3_003_FAN_HALF_SPEED
    [Documentation]    DAC 1650mV → DUT ADC ~50% → PWM duty ~50%
    Write DAC Voltage             5    DAC_OUT1    1650
    Sleep                         0.5
    PWM Read With Tolerance       6    PWM_IN1     1000    50    2000    10    10

TC_DUT3_004_FAN_FULL_SPEED
    [Documentation]    DAC 3000mV → DUT ADC ~91% → PWM duty ~91%
    Write DAC Voltage             7    DAC_OUT1    3000
    Sleep                         0.5
    PWM Read With Tolerance       8    PWM_IN1     1000    91    2000    10    10

TC_DUT3_005_FAN_LOW_SPEED
    [Documentation]    DAC 500mV → DUT ADC ~15% → PWM duty ~15%
    Write DAC Voltage             9    DAC_OUT1    500
    Sleep                         0.5
    PWM Read With Tolerance       10    PWM_IN1    1000    15    2000    10    10


# ── UART Echo ──────────────────────────────────────────────────────────────────

# ESP32 UART_CH1 TX (GPIO17) → STM32 PA3 (USART2 RX)

# DUT echoes received message back

# ESP32 UART_CH1 RX (GPIO16) ← STM32 PA2 (USART2 TX)

TC_DUT3_006_UART_ECHO_PING
    [Documentation]    Send PING over UART — DUT echoes PING back
    UART Send Expect    11    UART_CH1    PING    PING    1000

TC_DUT3_007_UART_ECHO_HELLO
    [Documentation]    Send HELLO over UART — DUT echoes HELLO back
    UART Send Expect    12    UART_CH1    HELLO    HELLO    1000


# ── SPI Write ──────────────────────────────────────────────────────────────────

# ESP32 sends command over SPI — validates transaction completes without error

# No readback — write only

# ESP32 SPI (GPIO18/19/23/5) → STM32 SPI1 (PA4/PA5/PA6/PA7)

TC_DUT3_008_SPI_LED_ON
    [Documentation]    Send LED_ON over SPI → STM32 sets PB5 HIGH → ESP32 reads DIO_IN2 HIGH
    SPI Write             13    LED_ON
    Sleep                 0.1
    Expect GPIO Value     14    DIO_IN2    1    500

TC_DUT3_009_SPI_LED_OFF
    [Documentation]    Send LED_OFF over SPI → STM32 sets PB5 LOW → ESP32 reads DIO_IN2 LOW
    SPI Write             15    LED_OFF
    Sleep                 0.1
    Expect GPIO Value     16    DIO_IN2    0    500


# ── PWM Stimulus → DUT Match ───────────────────────────────────────────────────

# ESP32 PWM_OUT1 (GPIO32) → STM32 PB4 (TIM3 CH1 input)

# DUT measures input freq/duty → reproduces on TIM1 PA8

# ESP32 PWM_IN1 (GPIO4) ← STM32 PA8

TC_DUT3_011_PWM_MATCH_100HZ_50PCT
    [Documentation]    ESP32 outputs 100Hz 50% → DUT matches → ESP32 validates
    PWM Write                  16    PWM_OUT1    100    50
    Sleep                      0.2
    PWM Read With Tolerance    17    PWM_IN2     100    50    2000    10    5

TC_DUT3_012_PWM_MATCH_200HZ_75PCT
    [Documentation]    ESP32 outputs 200Hz 75% → DUT matches → ESP32 validates
    PWM Write                  18    PWM_OUT1    200    75
    Sleep                      0.2
    PWM Read With Tolerance    19    PWM_IN2     200    75    2000    10    5
*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB1
${BAUDRATE}    115200

*** Test Cases ***

# ── PWM_WRITE ──────────────────────────────────────────────────────────────────

# Generate a PWM signal on PWM_OUT1 (GPIO32) using LEDC driver.

# Supports 1Hz to 100kHz.

#

# Frame:    ZTB|seq=1|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=100|duty_cycle=50

# Response: ZTB|seq=1|status=OK|freq_set=100|duty_set=50

TC_PWM_001_WRITE
    [Documentation]    Generate 100Hz 50% PWM on PWM_OUT1 (GPIO32)
    PWM Write    1    PWM_OUT1    100    50


# ── PWM_READ_WITH_TOLERANCE 50% ────────────────────────────────────────────────

# Write then validate on ESP32 side using GPIO interrupt capture on GPIO4.

# Requires wire from GPIO32 to GPIO4.

#

# Frame:    ZTB|seq=2|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=100|duty_cycle=50

#           ZTB|seq=3|cmd=PWM_READ_WITH_TOLERANCE|ch=PWM_IN1|frequency=100|duty_cycle=50|freq_tol_pct=5|duty_tol_pp=2|timeout=2000

# Response: ZTB|seq=3|status=OK|freq_expected=100|duty_expected=50|freq_measured=100|duty_measured=50|freq_tol_pct=5|duty_tol_pp=2

TC_PWM_002_WRITE_AND_VALIDATE_50PCT
    [Documentation]    Write 100Hz 50% then validate within tolerance
    PWM Write                  2    PWM_OUT1    100    50
    PWM Read With Tolerance    3    PWM_IN1     100    50    2000    5    2


# ── PWM_READ_WITH_TOLERANCE 75% ────────────────────────────────────────────────

TC_PWM_003_WRITE_AND_VALIDATE_75PCT
    [Documentation]    Write 100Hz 75% then validate within tolerance
    PWM Write                  4    PWM_OUT1    100    75
    PWM Read With Tolerance    5    PWM_IN1     100    75    2000    5    2


# ── PWM_READ_WITH_TOLERANCE 200Hz ──────────────────────────────────────────────

TC_PWM_004_WRITE_AND_VALIDATE_200HZ
    [Documentation]    Write 200Hz 50% then validate within tolerance
    PWM Write                  6    PWM_OUT1    200    50
    PWM Read With Tolerance    7    PWM_IN1     200    50    2000    5    2


# ── PWM_READ_WITH_TOLERANCE 1000Hz ─────────────────────────────────────────────

TC_PWM_005_WRITE_AND_VALIDATE_1000HZ
    [Documentation]    Write 1000Hz 25% then validate within tolerance
    PWM Write                  8    PWM_OUT1    1000    25
    PWM Read With Tolerance    9    PWM_IN1     1000    25    2000    5    2

TC_PWM_006_STM32_OUTPUT_100HZ_75PCT
    [Documentation]    Read PWM from STM32 output — expected 100Hz 75% on PWM_IN1 (GPIO4)
    ${response}=    PWM Read With Tolerance    10    PWM_IN1    1000    75    2000    5    2
    Log To Console    ${response}
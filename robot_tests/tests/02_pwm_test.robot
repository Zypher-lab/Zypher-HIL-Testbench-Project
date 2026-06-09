*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench
Library    BuiltIn

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***

# ── PWM_WRITE ──────────────────────────────────────────────────────────────────

# Generate a PWM signal on PWM_OUT1.

# MCPWM OUT0A drives GPIO32 through the ESP32 GPIO matrix.

#

# Frame sent:

#   ZTB|seq=1|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=100|duty_cycle=50

#

# Expected response:

#   ZTB|seq=1|status=OK|freq_set=100|duty_set=50

TC_PWM_001_WRITE
    [Documentation]    Generate 100Hz 50% PWM on PWM_OUT1 (GPIO32)
    PWM Write    1    PWM_OUT1    100    50
    Sleep        2

# ── PWM_READ ───────────────────────────────────────────────────────────────────

# Write then read back raw measured values.

# PWM_READ returns freq_measured and duty_measured — no validation on ESP32 side.

# Robot Framework decides pass/fail by checking the fields.

# Note: MCPWM measurement has ~1Hz rounding — 100Hz may read as 101Hz.

#

# Frames sent:

#   ZTB|seq=2|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=100|duty_cycle=50

#   ZTB|seq=3|cmd=PWM_READ|ch=PWM_IN1|timeout=2000

#

# Expected response:

#   ZTB|seq=3|status=OK|freq_measured=101|duty_measured=50

TC_PWM_002_READ
    [Documentation]    Write 100Hz 50% then read raw values back on PWM_IN1
    PWM Write             2    PWM_OUT1    100    50
    ${response}=    PWM Read    3    PWM_IN1    2000
    Response Field Should Equal    ${response}    freq_measured    101
    Response Field Should Equal    ${response}    duty_measured    50
    Sleep         2

# ── PWM_WRITE then PWM_READ ────────────────────────────────────────────────────

# Write 90% duty cycle then verify duty cycle measurement is exact.

#

# Frames sent:

#   ZTB|seq=4|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=100|duty_cycle=90

#   ZTB|seq=5|cmd=PWM_READ|ch=PWM_IN1|timeout=2000

#

# Expected response:

#   ZTB|seq=5|status=OK|freq_measured=101|duty_measured=90

TC_PWM_003_WRITE_THEN_READ
    [Documentation]    Write 100Hz 90% on OUT1 then read back on IN1
    PWM Write             4    PWM_OUT1    100    90
    Sleep             2
    ${response}=    PWM Read    5    PWM_IN1    2000
    Response Field Should Equal    ${response}    freq_measured    101
    Response Field Should Equal    ${response}    duty_measured    90


# ── PWM_READ_WITH_TOLERANCE ────────────────────────────────────────────────────

# Write then validate on ESP32 side with tolerance.

# Validation happens on the ESP32 — only OK or FAIL returned.

# freq_tol_pct=5 — frequency within 5% of expected

# duty_tol_pp=5  — duty within 5 percentage points

#

# Frame sent:

#   ZTB|seq=6|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=200|duty_cycle=75

#   ZTB|seq=7|cmd=PWM_READ_WITH_TOLERANCE|ch=PWM_IN1|frequency=200|duty_cycle=75|freq_tol_pct=5|duty_tol_pp=5|timeout=2000

#

# Expected response:

#   ZTB|seq=7|status=OK|freq_expected=200|duty_expected=75|freq_measured=202|duty_measured=75|freq_tol_pct=5|duty_tol_pp=5

TC_PWM_004_READ_WITH_TOLERANCE
    [Documentation]    Write 200Hz 75% on OUT1 then validate on IN1 within tolerance
    PWM Write                  6    PWM_OUT1    100    75
    Sleep        2
    PWM Read With Tolerance    7    PWM_IN1     100    75    5000    10    10
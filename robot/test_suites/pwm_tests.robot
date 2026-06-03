*** Settings ***
Library    ../resources/serial_lib.py

Suite Setup     open_bench    ${CONFIG}
Suite Teardown  close_bench

*** Variables ***
${CONFIG}    ${CURDIR}/../config/bench.yaml

*** Test Cases ***

PWM 50Hz 90 Duty
    ${resp}=    send_command    SET:PWM:PA8:50:90|ASSERT:PWM:PB10:50:90|TIMEOUT:2000
    response_should_be_ok    ${resp}

PWM 100Hz 50 Duty
    ${resp}=    send_command    SET:PWM:PA8:100:50|ASSERT:PWM:PB10:100:50|TIMEOUT:2000
    response_should_be_ok    ${resp}

PWM 1000Hz 10 Duty
    ${resp}=    send_command    SET:PWM:PA8:1000:10|ASSERT:PWM:PB10:1000:10|TIMEOUT:2000
    response_should_be_ok    ${resp}
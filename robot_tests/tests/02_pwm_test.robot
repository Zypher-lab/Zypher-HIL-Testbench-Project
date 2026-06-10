*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup       Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***

TC_1_PWM_MATCH_100HZ_50PCT
    [Documentation]    ESP32 outputs 100Hz 50% → DUT matches → ESP32 validates
    PWM Write                  16    PWM_OUT1    100    50
    Sleep                      0.2
    PWM Read With Tolerance    17    PWM_IN2     100    50    2000    10    5

TC_2_PWM_MATCH_200HZ_75PCT
    [Documentation]    ESP32 outputs 200Hz 75% → DUT matches → ESP32 validates
    PWM Write                  18    PWM_OUT1    200    75
    Sleep                      0.2
    PWM Read With Tolerance    19    PWM_IN2     200    75    2000    10    5
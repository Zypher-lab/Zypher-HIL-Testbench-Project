*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup    Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***
TC_DAC_OUT2_001_LOW_VOLTAGE_WARNING_OFF
    Write DAC Voltage    1    DAC_OUT2    0
    Expect GPIO Value    2    DIO_IN2     0    1000

TC_DAC_OUT2_002_HIGH_VOLTAGE_WARNING_ON
    Write DAC Voltage    3    DAC_OUT2    2500
    Expect GPIO Value    4    DIO_IN2     1    1000

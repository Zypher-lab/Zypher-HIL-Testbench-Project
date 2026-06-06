*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup    Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***
TC_ADC_001_LOW_VOLTAGE_WARNING_OFF
    Write DAC Voltage    1    DAC_OUT1    0
    Expect GPIO Value    2    DIO_IN2     0    1000

TC_ADC_002_HIGH_VOLTAGE_WARNING_ON
    Write DAC Voltage    3    DAC_OUT1    2500
    Expect GPIO Value    4    DIO_IN2     1    1000
TC_ADC_003_LOW_VOLTAGE_WARNING_OFF
    Write DAC Voltage    1    DAC_OUT1    1300
    Expect GPIO Value    2    DIO_IN2     0    1000F
TC_ADC_004_HIGH_VOLTAGE_WARNING_ON
    Write DAC Voltage    3    DAC_OUT1    3100
    Expect GPIO Value    4    DIO_IN2     1    1000
TC_ADC_005_THRESHOLD
    Write DAC Voltage    3    DAC_OUT1    1500
    Expect GPIO Value    4    DIO_IN2     1    1000


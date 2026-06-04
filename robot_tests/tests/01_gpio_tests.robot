*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup    Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***
TC_DIO_001_BUTTON_HIGH_LED_ON
    Send GPIO Write    1    DIO_OUT1    1
    Expect GPIO Value    2    DIO_IN1     1    500

TC_DIO_002_BUTTON_LOW_LED_OFF
    Send GPIO Write    3    DIO_OUT1    0
    Expect GPIO Value    4    DIO_IN1     0    500
TC_DIO_003_OUT2_HIGH_IN2_HIGH
    Send GPIO Write    5    DIO_OUT2    1
    Expect GPIO Value    6    DIO_IN2     1    500
TC_DIO_004_OUT2_LOW_IN2_LOW
    Send GPIO Write    7    DIO_OUT2    0
    Expect GPIO Value    8    DIO_IN2     0    500
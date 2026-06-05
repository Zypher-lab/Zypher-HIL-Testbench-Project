*** Settings ***
Resource    ../resources/common_keywords.robot
Suite Setup    Open Testbench    ${PORT}    ${BAUDRATE}
Suite Teardown    Close Testbench

*** Variables ***
${PORT}        /dev/ttyUSB0
${BAUDRATE}    115200

*** Test Cases ***
TC_UART_001_SEND_EXPECT_PING_PONG
    UART Send Expect    1    UART_CH1    PING    PONG    1000

TC_UART_002_WRITE_READ_HELLO_ACK
    Write UART Text             2    UART_CH1    HELLO
    Read UART Text Should Be    3    UART_CH1    ACK    1000

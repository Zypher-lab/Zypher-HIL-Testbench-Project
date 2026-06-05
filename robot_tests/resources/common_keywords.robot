*** Settings ***
Library    ../libraries/TestbenchSerial.py

*** Keywords ***
Send GPIO Write
    [Arguments]    ${seq}    ${channel}    ${value}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=GPIO_WRITE|ch=${channel}|val=${value}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

Expect GPIO Value
    [Arguments]    ${seq}    ${channel}    ${value}    ${timeout}=500
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=GPIO_EXPECT|ch=${channel}|val=${value}|timeout=${timeout}    ${seq}
    Response Status Should Be OK    ${response}
    Response Field Should Equal    ${response}    expected    ${value}
    Response Field Should Equal    ${response}    actual    ${value}
    RETURN    ${response}

Write DAC Voltage
    [Arguments]    ${seq}    ${channel}    ${mv}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=DAC_WRITE|ch=${channel}|mv=${mv}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

Write UART Text
    [Arguments]    ${seq}    ${channel}    ${tx}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=UART_WRITE|ch=${channel}|tx=${tx}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

Read UART Text Should Be
    [Arguments]    ${seq}    ${channel}    ${expected_rx}    ${timeout}=1000
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=UART_READ|ch=${channel}|timeout=${timeout}    ${seq}
    Response Status Should Be OK    ${response}
    Should Contain    ${response}    rx=${expected_rx}
    RETURN    ${response}

UART Send Expect
    [Arguments]    ${seq}    ${channel}    ${tx}    ${expected_rx}    ${timeout}=1000
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=UART_SEND_EXPECT|ch=${channel}|tx=${tx}|expect=${expected_rx}|timeout=${timeout}    ${seq}
    Response Status Should Be OK    ${response}
    Should Contain    ${response}    rx=${expected_rx}
    RETURN    ${response}

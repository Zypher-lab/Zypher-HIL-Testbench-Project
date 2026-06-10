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
PWM Write
    [Arguments]    ${seq}    ${channel}    ${frequency}    ${duty_cycle}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=PWM_WRITE|ch=${channel}|frequency=${frequency}|duty_cycle=${duty_cycle}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

PWM Read
    [Arguments]    ${seq}    ${channel}    ${timeout}=1000
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=PWM_READ|ch=${channel}|timeout=${timeout}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

PWM Read With Tolerance
    [Arguments]    ${seq}    ${channel}    ${frequency}    ${duty_cycle}    ${timeout}=1000    ${freq_tol_pct}=2    ${duty_tol_pp}=2
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=PWM_READ_WITH_TOLERANCE|ch=${channel}|frequency=${frequency}|duty_cycle=${duty_cycle}|freq_tol_pct=${freq_tol_pct}|duty_tol_pp=${duty_tol_pp}|timeout=${timeout}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}

SPI Send Expect
    [Arguments]    ${seq}    ${tx}    ${expected_rx}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=SPI_SEND_EXPECT|tx=${tx}|expect=${expected_rx}    ${seq}
    Response Status Should Be OK    ${response}
    Should Contain    ${response}    rx=${expected_rx}
    RETURN    ${response}
SPI Write
    [Arguments]    ${seq}    ${tx}
    ${response}=    Send ZTB Command    ZTB|seq=${seq}|cmd=SPI_WRITE|tx=${tx}    ${seq}
    Response Status Should Be OK    ${response}
    RETURN    ${response}
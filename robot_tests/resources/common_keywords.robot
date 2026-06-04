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

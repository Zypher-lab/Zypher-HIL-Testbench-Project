# ZTB Serial Protocol

Robot Framework communicates with the ESP32 testbench over USB serial.
All commands and responses are line-based text frames.

---

## Frame Format

Command format:
```text
ZTB|seq=<number>|cmd=<command>|key=value|...
```

Response format:
```text
ZTB|seq=<number>|status=OK|...
ZTB|seq=<number>|status=FAIL|err=<reason>
```

---

## Supported Commands

### GPIO_WRITE
Drive a digital output resource.
```text
ZTB|seq=1|cmd=GPIO_WRITE|ch=DIO_OUT1|val=1
```
Example response:
```text
ZTB|seq=1|status=OK
```

---

### GPIO_READ
Read a digital input or output resource.
```text
ZTB|seq=2|cmd=GPIO_READ|ch=DIO_IN1
```
Example response:
```text
ZTB|seq=2|status=OK|val=1
```

---

### GPIO_EXPECT
Wait until a digital input matches the expected value or timeout expires.
```text
ZTB|seq=3|cmd=GPIO_EXPECT|ch=DIO_IN1|val=1|timeout=500
```
Example pass response:
```text
ZTB|seq=3|status=OK|expected=1|actual=1
```
Example fail response:
```text
ZTB|seq=3|status=FAIL|expected=1|actual=0|err=GPIO_MISMATCH
```

---

### DAC_WRITE
Set an analog voltage using an ESP32 DAC output.
```text
ZTB|seq=4|cmd=DAC_WRITE|ch=DAC_OUT1|mv=2500
ZTB|seq=5|cmd=DAC_WRITE|ch=DAC_OUT2|mv=2500
```
Example response:
```text
ZTB|seq=4|status=OK
```

---

### UART_WRITE
Send text over a UART resource.
```text
ZTB|seq=6|cmd=UART_WRITE|ch=UART_CH1|tx=PING
```
Example response:
```text
ZTB|seq=6|status=OK
```

---

### UART_READ
Read one line from a UART resource until newline or timeout.
```text
ZTB|seq=7|cmd=UART_READ|ch=UART_CH1|timeout=1000
```
Example response:
```text
ZTB|seq=7|status=OK|rx=PONG
```
Timeout response:
```text
ZTB|seq=7|status=FAIL|err=UART_READ_TIMEOUT
```

---

### UART_SEND_EXPECT
Send text over UART, read the response, and compare it with the expected value.
```text
ZTB|seq=8|cmd=UART_SEND_EXPECT|ch=UART_CH1|tx=PING|expect=PONG|timeout=1000
```
Example pass response:
```text
ZTB|seq=8|status=OK|rx=PONG
```
Example fail response:
```text
ZTB|seq=8|status=FAIL|rx=ERR|err=UART_MISMATCH
```

---

### PWM_WRITE
Generate a PWM signal on an output pin.
The ESP32 drives the pin at the specified frequency and duty cycle.
Use this to stimulate a DUT input with a known PWM signal.
```text
ZTB|seq=9|cmd=PWM_WRITE|ch=PWM_OUT1|frequency=50|duty_cycle=90
```
Example response:
```text
ZTB|seq=9|status=OK|freq_set=50|duty_set=50
```
Example fail response:
```text
ZTB|seq=9|status=FAIL|err=PWM_WRITE_FAIL
```

---

### PWM_READ_WITH_TOLERANCE
Measure a PWM signal on a capture pin and validate it against expected values within tolerance.
Validation happens on the ESP32 — only OK or FAIL is returned.

- `freq_tol_pct` — frequency tolerance as a percentage of expected value (default: 2)
- `duty_tol_pp`  — duty cycle tolerance in absolute percentage points (default: 2)

```text
ZTB|seq=11|cmd=PWM_READ_WITH_TOLERANCE|ch=PWM_IN1|frequency=50|duty_cycle=90|freq_tol_pct=2|duty_tol_pp=2|timeout=1000
```
Example pass response:
```text
ZTB|seq=11|status=OK|freq_expected=50|duty_expected=90|freq_measured=50|duty_measured=91|freq_tol_pct=2|duty_tol_pp=2
```
Example fail response:
```text
ZTB|seq=11|status=FAIL|freq_expected=50|duty_expected=90|freq_measured=35|duty_measured=91|freq_tol_pct=2|duty_tol_pp=2|err=PWM_MISMATCH
```

---

### SPI_WRITE
Send a command byte over SPI to a slave device.
Write-only — no response data is read back from MISO.
Use this to send configuration commands to a DUT SPI slave and validate the effect through a GPIO or other peripheral.

Supported commands:
- `LED_ON`  → sends 0x01
- `LED_OFF` → sends 0x00

```text
ZTB|seq=12|cmd=SPI_WRITE|tx=LED_ON
```
Example pass response:
```text
ZTB|seq=12|status=OK
```
Example fail response:
```text
ZTB|seq=12|status=FAIL|err=SPI_WRITE_FAILED
```
---
### SPI_SEND_EXPECT
Send bytes over SPI and validate the received response.
Full duplex — ESP32 clocks out `tx` while simultaneously reading `rx`, then compares against `expect`.
Use this for loopback tests (MOSI wired to MISO) or bidirectional SPI devices.

```text
ZTB|seq=13|cmd=SPI_SEND_EXPECT|tx=PING|expect=PING
```
Example pass response:
```text
ZTB|seq=13|status=OK|rx=PING
```
Example fail response:
```text
ZTB|seq=13|status=FAIL|rx=PONG|err=SPI_MISMATCH
```
---

## Current Logical Resources

| Resource  | Type           | Physical Pin                    |
|-----------|----------------|---------------------------------|
| DIO_OUT1  | Digital output | ESP32 GPIO27                    |
| DIO_OUT2  | Digital output | ESP32 GPIO13                    |
| DIO_IN1   | Digital input  | ESP32 GPIO14                    |
| DIO_IN2   | Digital input  | ESP32 GPIO22                    |
| DAC_OUT1  | DAC output     | ESP32 GPIO25                    |
| DAC_OUT2  | DAC output     | ESP32 GPIO26                    |
| UART_CH1  | UART           | ESP32 GPIO17 TXD2 / GPIO16 RXD2 |
| PWM_OUT1  | PWM output     | ESP32 GPIO32                    |
| PWM_IN1   | PWM input      | ESP32 GPIO34                    |
| PWM_IN2   | PWM input      | ESP32 GPIO2                     |
| SPI_CLK     | GPIO18 | ESP32 → STM32-2 | STM32-2 PA5 (SCK)   | SPI        |
| SPI_MOSI    | GPIO23 | ESP32 → STM32-2 | STM32-2 PA7 (MOSI)  | SPI        |
| SPI_MISO    | GPIO19 | STM32-2 → ESP32 | STM32-2 PA6 (MISO)  | SPI        |
| SPI_CS      | GPIO5  | ESP32 → STM32-2 | STM32-2 PA4 (NSS)   | SPI        |

---

## Notes

- Robot Framework must use logical resource names only.
- Physical pin mapping is owned by ESP32 `board_map.c` and service modules.
- `DAC_OUT1` and `DAC_OUT2` are used to stimulate STM32 ADC input.
- `DIO_IN2` is used as the ADC warning monitor in the current ADC E2E test.
- `UART_CH1` is used for STM32 UART DUT E2E validation.
- `UART_WRITE` and `UART_READ` are generic UART commands.
- `UART_SEND_EXPECT` is a helper command for complete UART request-response validation.
- `PWM_OUT1` and `PWM_OUT2` connect to STM32-2 PWM capture inputs (PA0, PA1).
- `PWM_IN1` and `PWM_IN2` connect to STM32-2 PWM outputs (PA8, PB4).
- Tolerance fields are optional in `PWM_READ_WITH_TOLERANCE` — defaults are `freq_tol_pct=2` and `duty_tol_pp=2`.
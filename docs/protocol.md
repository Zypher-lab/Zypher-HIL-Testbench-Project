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

## Current Logical Resources

| Resource | Type | Physical Pin |
|---|---|---|
| DIO_OUT1 | Digital output | ESP32 GPIO27 |
| DIO_OUT2 | Digital output | ESP32 GPIO13 |
| DIO_IN1 | Digital input | ESP32 GPIO14 |
| DIO_IN2 | Digital input | ESP32 GPIO22 |
| DAC_OUT1 | DAC output | ESP32 GPIO25 |
| DAC_OUT2 | DAC output | ESP32 GPIO26 |
| UART_CH1 | UART | ESP32 GPIO17 TXD2 / GPIO16 RXD2 |

---

## Notes

- Robot Framework must use logical resource names only.
- Physical pin mapping is owned by ESP32 `board_map.c` and service modules.
- `DAC_OUT1` and `DAC_OUT2` are used to stimulate STM32 ADC input.
- `DIO_IN2` is used as the ADC warning monitor in the current ADC E2E test.
- `UART_CH1` is used for STM32 UART DUT E2E validation.
- `UART_WRITE` and `UART_READ` are generic UART commands.
- `UART_SEND_EXPECT` is a helper command for complete UART request-response validation.

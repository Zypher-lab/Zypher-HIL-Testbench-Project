# Wiring Guide

This document defines the validated wiring between the ESP32 testbench and the STM32 DUT.

---

## Common Requirement

All boards must share a common ground.

```text
ESP32 GND -> STM32 GND
```

---

## DIO E2E Wiring

| ESP32 Testbench Resource | ESP32 Pin | STM32 DUT Pin | Direction | Purpose |
|---|---|---|---|---|
| DIO_OUT1 | GPIO27 | PB0 | ESP32 -> STM32 | Simulated button/input |
| DIO_IN1 | GPIO14 | PB1 | STM32 -> ESP32 | DUT LED/output monitor |

DUT behavior:

```text
PB0 LOW  -> PB1 LOW
PB0 HIGH -> PB1 HIGH
```

---

## Multi-channel DIO Loopback Validation

For ESP32-only DIO resource validation:

| Output Resource | ESP32 Output Pin | Input Resource | ESP32 Input Pin |
|---|---|---|---|
| DIO_OUT1 | GPIO27 | DIO_IN1 | GPIO14 |
| DIO_OUT2 | GPIO13 | DIO_IN2 | GPIO22 |

---

## ADC E2E Wiring

| ESP32 Testbench Resource | ESP32 Pin | STM32 DUT Pin | Direction | Purpose |
|---|---|---|---|---|
| DAC_OUT1 | GPIO25 | PA0 | ESP32 -> STM32 | Analog stimulus to STM32 ADC |
| DAC_OUT2 | GPIO26 | PA0 | ESP32 -> STM32 | Alternative analog stimulus to STM32 ADC |
| DIO_IN2 | GPIO22 | PB5 | STM32 -> ESP32 | ADC warning output monitor |

DUT behavior:

```text
PA0 voltage < threshold  -> PB5 LOW
PA0 voltage >= threshold -> PB5 HIGH
```

Current threshold:

```text
ADC raw threshold = 1800
Approximately around 1.5V on a 12-bit 3.3V ADC scale
```

---

## UART E2E Wiring

| ESP32 Testbench Resource | ESP32 Pin | STM32 DUT Pin | Direction | Purpose |
|---|---|---|---|---|
| UART_CH1 TX | GPIO17 / TXD2 | PA3 / USART2_RX | ESP32 -> STM32 | UART request to DUT |
| UART_CH1 RX | GPIO16 / RXD2 | PA2 / USART2_TX | STM32 -> ESP32 | UART response from DUT |

UART wiring rule:

```text
ESP32 TX -> STM32 RX
ESP32 RX <- STM32 TX
ESP32 GND -> STM32 GND
```

Validated DUT behavior:

```text
PING  -> PONG
HELLO -> ACK
```

---

## ST-Link Wiring for STM32 Flashing

| ST-Link | STM32 |
|---|---|
| SWDIO | PA13 |
| SWCLK | PA14 |
| GND | GND |
| 3.3V / VCC | 3.3V |
| NRST | NRST |

NRST is required for reliable flashing with OpenOCD.


# Wiring Guide

All boards must share a common ground.

```
ESP32 GND → STM32-1 GND
ESP32 GND → STM32-2 GND
STM32-1 GND → STM32-2 GND
```

---

## Pin Allocation Summary

### ESP32 DevKitC — Full Resource Map

| Resource    | GPIO   | Direction     | Connected to          | Peripheral |
|-------------|--------|---------------|-----------------------|------------|
| DIO_OUT1    | GPIO27 | ESP32 → STM32-1 | STM32-1 PB0         | GPIO       |
| DIO_OUT2    | GPIO13 | ESP32 → STM32-1 | STM32-1 PB1         | GPIO       |
| DIO_IN1     | GPIO14 | STM32-1 → ESP32 | STM32-1 PB2         | GPIO       |
| DIO_IN2     | GPIO22 | STM32-1 → ESP32 | STM32-1 PB3         | GPIO       |
| DAC_OUT1    | GPIO25 | ESP32 → STM32-1 | STM32-1 PA0 (ADC)   | DAC        |
| DAC_OUT2    | GPIO26 | ESP32 → STM32-1 | STM32-1 PA1 (ADC)   | DAC        |
| UART TX     | GPIO17 | ESP32 → STM32-1 | STM32-1 PA3 (RX)    | UART       |
| UART RX     | GPIO16 | STM32-1 → ESP32 | STM32-1 PA2 (TX)    | UART       |
| PWM_OUT1    | GPIO32 | ESP32 → STM32-2 | STM32-2 PA0 (TIM2)  | LEDC       |
| PWM_OUT2    | GPIO33 | ESP32 → STM32-2 | STM32-2 PA1 (TIM2)  | LEDC       |
| PWM_IN1     | GPIO34 | STM32-2 → ESP32 | STM32-2 PA8 (TIM1)  | MCPWM      |
| PWM_IN2     | GPIO35 | STM32-2 → ESP32 | STM32-2 PB4 (TIM3)  | MCPWM      |
| SPI_CLK     | GPIO18 | ESP32 → STM32-2 | STM32-2 PA5 (SCK)   | SPI        |
| SPI_MOSI    | GPIO23 | ESP32 → STM32-2 | STM32-2 PA7 (MOSI)  | SPI        |
| SPI_MISO    | GPIO19 | STM32-2 → ESP32 | STM32-2 PA6 (MISO)  | SPI        |
| SPI_CS      | GPIO5  | ESP32 → STM32-2 | STM32-2 PA4 (NSS)   | SPI        |

---

## STM32-1 — GPIO, DAC/ADC, UART

### DIO E2E Wiring

| ESP32 Resource | ESP32 Pin | STM32-1 Pin | Direction       |
|----------------|-----------|-------------|-----------------|
| DIO_OUT1       | GPIO27    | PB0         | ESP32 → STM32-1 |
| DIO_OUT2       | GPIO13    | PB1         | ESP32 → STM32-1 |
| DIO_IN1        | GPIO14    | PB2         | STM32-1 → ESP32 |
| DIO_IN2        | GPIO22    | PB3         | STM32-1 → ESP32 |

DUT behavior:
```
PB0 LOW  → PB2 LOW
PB0 HIGH → PB2 HIGH
```

### DIO Loopback (ESP32 self-test)

| Output   | ESP32 Pin | Input   | ESP32 Pin |
|----------|-----------|---------|-----------|
| DIO_OUT1 | GPIO27    | DIO_IN1 | GPIO14    |
| DIO_OUT2 | GPIO13    | DIO_IN2 | GPIO22    |

### DAC / ADC E2E Wiring

| ESP32 Resource | ESP32 Pin | STM32-1 Pin | Direction       |
|----------------|-----------|-------------|-----------------|
| DAC_OUT1       | GPIO25    | PA0 (ADC1)  | ESP32 → STM32-1 |
| DAC_OUT2       | GPIO26    | PA1 (ADC1)  | ESP32 → STM32-1 |
| DIO_IN1        | GPIO14    | PB2         | STM32-1 → ESP32 |

DUT behavior:
```
PA0 voltage < threshold  → PB2 LOW
PA0 voltage >= threshold → PB2 HIGH
```

### UART E2E Wiring

| ESP32 Resource | ESP32 Pin     | STM32-1 Pin     | Direction       |
|----------------|---------------|-----------------|-----------------|
| UART_CH1 TX    | GPIO17 (TXD2) | PA3 (USART2_RX) | ESP32 → STM32-1 |
| UART_CH1 RX    | GPIO16 (RXD2) | PA2 (USART2_TX) | STM32-1 → ESP32 |

```
ESP32 TX → STM32-1 RX
ESP32 RX ← STM32-1 TX
```

Validated DUT behavior:
```
PING  → PONG
HELLO → ACK
```

---

## STM32-2 — PWM, SPI

### PWM E2E Wiring

| ESP32 Resource | ESP32 Pin | STM32-2 Pin    | Direction       | Peripheral             |
|----------------|-----------|----------------|-----------------|------------------------|
| PWM_OUT1       | GPIO32    | PA0 (TIM2 CH1) | ESP32 → STM32-2 | LEDC out → TIM2 input  |
| PWM_OUT2       | GPIO33    | PA1 (TIM2 CH2) | ESP32 → STM32-2 | LEDC out → TIM2 input  |
| PWM_IN1        | GPIO34    | PA8 (TIM1 CH1) | STM32-2 → ESP32 | TIM1 out → MCPWM input |
| PWM_IN2        | GPIO35    | PB4 (TIM3 CH1) | STM32-2 → ESP32 | TIM3 out → MCPWM input |

PWM test cases:
```
PWM_WRITE:  ESP32 generates PWM on GPIO32/33 → STM32-2 measures on PA0/PA1
PWM_READ:   STM32-2 generates PWM on PA8/PB4 → ESP32 measures on GPIO34/35
PWM_EXPECT: measure + validate against expected freq/duty with tolerance
```

### SPI E2E Wiring

| ESP32 Resource | ESP32 Pin | STM32-2 Pin  | Direction       |
|----------------|-----------|--------------|-----------------|
| SPI_CLK        | GPIO18    | PA5 (SCK)    | ESP32 → STM32-2 |
| SPI_MOSI       | GPIO23    | PA7 (MOSI)   | ESP32 → STM32-2 |
| SPI_MISO       | GPIO19    | PA6 (MISO)   | STM32-2 → ESP32 |
| SPI_CS         | GPIO5     | PA4 (NSS)    | ESP32 → STM32-2 |

```
ESP32 = SPI master
STM32-2 = SPI slave
```

---

## ST-Link Wiring

### STM32-1

| ST-Link | STM32-1 |
|---------|---------|
| SWDIO   | PA13    |
| SWCLK   | PA14    |
| GND     | GND     |
| 3.3V    | 3.3V    |
| NRST    | NRST    |

### STM32-2

| ST-Link | STM32-2 |
|---------|---------|
| SWDIO   | PA13    |
| SWCLK   | PA14    |
| GND     | GND     |
| 3.3V    | 3.3V    |
| NRST    | NRST    |

---

## Voltage Levels

| Signal      | Level  | Note                                  |
|-------------|--------|---------------------------------------|
| ESP32 GPIO  | 3.3V   | All GPIO are 3.3V                     |
| STM32 GPIO  | 3.3V   | F401 runs at 3.3V                     |
| DAC output  | 0-3.3V | ESP32 DAC max is 3.3V                 |
| PWM signals | 3.3V   | Compatible — no level shifting needed |
| SPI signals | 3.3V   | Compatible — no level shifting needed |
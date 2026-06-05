# Wiring Guide

This document defines the validated wiring between the ESP32 testbench and the STM32 DUT.

---

## Common Requirement

All boards must share a common ground.

```text
ESP32 GND -> STM32 GND
````

---

## DIO E2E Wiring

| ESP32 Testbench Resource | ESP32 Pin | STM32 DUT Pin | Direction      | Purpose                |
| ------------------------ | --------- | ------------- | -------------- | ---------------------- |
| DIO_OUT1                 | GPIO27    | PB0           | ESP32 -> STM32 | Simulated button/input |
| DIO_IN1                  | GPIO14    | PB1           | STM32 -> ESP32 | DUT LED/output monitor |

DUT behavior:

```text
PB0 LOW  -> PB1 LOW
PB0 HIGH -> PB1 HIGH
```

---

## Multi-channel DIO Loopback Validation

For ESP32-only DIO resource validation:

| Output Resource | ESP32 Output Pin | Input Resource | ESP32 Input Pin |
| --------------- | ---------------- | -------------- | --------------- |
| DIO_OUT1        | GPIO27           | DIO_IN1        | GPIO14          |
| DIO_OUT2        | GPIO13           | DIO_IN2        | GPIO22          |

---

## ADC E2E Wiring

| ESP32 Testbench Resource | ESP32 Pin | STM32 DUT Pin | Direction      | Purpose                      |
| ------------------------ | --------- | ------------- | -------------- | ---------------------------- |
| DAC_OUT1                 | GPIO25    | PA0           | ESP32 -> STM32 | Analog stimulus to STM32 ADC |
| DIO_IN2                  | GPIO22    | PB5           | STM32 -> ESP32 | ADC warning output monitor   |

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

## ST-Link Wiring for STM32 Flashing

| ST-Link    | STM32 |
| ---------- | ----- |
| SWDIO      | PA13  |
| SWCLK      | PA14  |
| GND        | GND   |
| 3.3V / VCC | 3.3V  |
| NRST       | NRST  |

NRST is required for reliable flashing with OpenOCD.
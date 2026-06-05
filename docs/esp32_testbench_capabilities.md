# ESP32 Testbench Capability Sheet

This document defines the logical resources exposed by the ESP32 testbench firmware.

Robot Framework must use logical resource names such as `DIO_OUT1` or `DAC_OUT1`, not physical GPIO numbers.
The physical pin mapping is owned by the ESP32 testbench firmware.

---

## Current Supported Resources

| Resource | Type | Direction | ESP32 Pin | Description |
|---|---|---|---|---|
| DIO_OUT1 | Digital GPIO | Output | GPIO27 | Digital stimulus output 1 |
| DIO_OUT2 | Digital GPIO | Output | GPIO13 | Digital stimulus output 2 |
| DIO_IN1 | Digital GPIO | Input | GPIO14 | Digital monitor input 1 |
| DIO_IN2 | Digital GPIO | Input | GPIO22 | Digital monitor input 2 |
| DAC_OUT1 | DAC | Output | GPIO25 | Analog voltage stimulus output |

---

## Validated Testbench Capabilities

### DIO

| Test | Connection | Result |
|---|---|---|
| DIO_OUT1 -> DIO_IN1 | GPIO27 -> GPIO14 | Passed |
| DIO_OUT2 -> DIO_IN2 | GPIO13 -> GPIO22 | Passed |

### ADC Stimulus

| Resource | ESP32 Pin | Connected DUT Pin | Result |
|---|---|---|---|
| DAC_OUT1 | GPIO25 | STM32 PA0 ADC_IN | Passed |
| DIO_IN2 | GPIO22 | STM32 PB5 WARNING_OUT | Passed |

---

## Reserved Pins

| ESP32 Pin(s) | Purpose |
|---|---|
| GPIO1 / GPIO3 | USB serial, flashing, console, and ZTB protocol |
| GPIO25 | DAC_OUT1 analog stimulus |
| GPIO26 | Reserved for future DAC_OUT2 |
| GPIO16 / GPIO17 | Reserved for UART test interface |
| GPIO18 / GPIO19 / GPIO21 / GPIO23 | Reserved for SPI test interface |
| GPIO6 - GPIO11 | Usually connected to internal SPI flash |
| GPIO0 / GPIO2 / GPIO5 / GPIO12 / GPIO15 | Boot strapping pins; avoid unless validated |
| GPIO34 - GPIO39 | Input-only pins, no software pull-up/pull-down |

---

## Notes

- All boards must share a common GND.
- Robot Framework communicates only with the ESP32 over USB serial.
- STM32 is treated as the DUT and reacts only to physical signals.
- The ESP32 testbench firmware is generic and executes ZTB protocol commands.
- `DAC_OUT1` uses the real ESP32 DAC driver enabled through `app.overlay`.

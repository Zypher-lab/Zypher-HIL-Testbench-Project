# ESP32 Testbench Capability Sheet

This document defines the logical resources exposed by the ESP32 testbench firmware.

Robot Framework must use logical resource names such as `DIO_OUT1`, not physical GPIO numbers.
The physical pin mapping is owned by the ESP32 testbench firmware `board_map.c`.

---

## Current Supported DIO Resources

| Resource | Direction | ESP32 Pin | Description |
|---|---|---|---|
| DIO_OUT1 | Output | GPIO27 | Digital stimulus output 1 |
| DIO_OUT2 | Output | GPIO13 | Digital stimulus output 2 |
| DIO_IN1 | Input | GPIO14 | Digital monitor input 1 |
| DIO_IN2 | Input | GPIO22 | Digital monitor input 2 |

---

## Reserved Pins

| ESP32 Pin(s) | Purpose |
|---|---|
| GPIO1 / GPIO3 | USB serial, flashing, console, and ZTB protocol |
| GPIO25 / GPIO26 | DAC/ADC stimulus |
| GPIO16 / GPIO17 | UART test interface |
| GPIO18 / GPIO19 / GPIO21 / GPIO23 | SPI test interface |
| GPIO6 - GPIO11 | Usually connected to internal SPI flash |
| GPIO0 / GPIO2 / GPIO5 / GPIO12 / GPIO15 | Boot strapping pins; avoid unless validated |
| GPIO34 - GPIO39 | Input-only pins, no software pull-up/pull-down |

---

## DIO Loopback Validation

| Test | Connection | Result |
|---|---|---|
| DIO_OUT1 -> DIO_IN1 | GPIO27 -> GPIO14 | Passed |
| DIO_OUT2 -> DIO_IN2 | GPIO13 -> GPIO22 | Passed |

---

## Notes

- All boards must share a common GND.
- Robot Framework communicates only with the ESP32 over USB serial.
- STM32 is treated as the DUT and reacts only to physical signals.
- The ESP32 testbench firmware is generic and executes commands received through the ZTB protocol.

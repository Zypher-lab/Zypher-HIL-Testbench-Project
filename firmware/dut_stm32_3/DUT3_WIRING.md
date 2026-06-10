# DUT3 Wiring Guide

Complete physical connection reference between the ESP32 testbench and STM32 Black Pill DUT3.

---

## Required Hardware

- ESP32 DevKit (testbench)
- STM32F401CC Black Pill (DUT3)
- 13 jumper wires
- Common ground wire

---

## Connection Table

| ESP32 Pin | Direction | STM32 Pin | Signal       | Peripheral      |
|-----------|-----------|-----------|--------------|-----------------|
| GPIO27    | →         | PB0       | DIO_OUT1     | GPIO input      |
| GPIO14    | ←         | PB1       | DIO_IN1      | GPIO output     |
| GPIO25    | →         | PA0       | DAC_OUT1     | ADC1 channel 0  |
| GPIO17    | →         | PA3       | UART TX→RX   | USART2 RX       |
| GPIO16    | ←         | PA2       | UART RX←TX   | USART2 TX       |
| GPIO5     | →         | PA4       | SPI CS       | SPI1 NSS        |
| GPIO18    | →         | PA5       | SPI SCK      | SPI1 SCK        |
| GPIO23    | →         | PA7       | SPI MOSI     | SPI1 MOSI       |
| GPIO32    | →         | PB4       | PWM_OUT1     | GPIO ISR input  |
| GPIO4     | ←         | PA8       | PWM_IN1      | TIM1 CH1 output |
| GPIO2     | ←         | PB10      | PWM_IN2      | TIM2 CH3 output |
| GPIO22    | ←         | PB5       | DIO_IN2      | GPIO output     |
| GND       | —         | GND       | Common ground | —              |

---

## Wiring by Peripheral

### GPIO — Switch / LED
```
ESP32 GPIO27  ──────────────►  STM32 PB0   (switch input)
ESP32 GPIO14  ◄──────────────  STM32 PB1   (LED output)
```
PB0 drives PB1 — simulates a switch controlling an LED.

---

### DAC / ADC — Fan Speed
```
ESP32 GPIO25  ──────────────►  STM32 PA0   (analog voltage 0–3.3V)
```
ESP32 DAC outputs a voltage. STM32 ADC reads it and sets PWM duty proportionally.

---

### PWM — Fan Output
```
ESP32 GPIO4   ◄──────────────  STM32 PA8   (TIM1 CH1 — fan PWM at 1kHz)
```
ESP32 captures the fan PWM and validates frequency and duty cycle.

---

### UART — Echo
```
ESP32 GPIO17  ──────────────►  STM32 PA3   (USART2 RX)
ESP32 GPIO16  ◄──────────────  STM32 PA2   (USART2 TX)
```
Cross-connect TX→RX and RX→TX. STM32 echoes every received message.

---

### SPI — Command
```
ESP32 GPIO5   ──────────────►  STM32 PA4   (SPI1 NSS / CS)
ESP32 GPIO18  ──────────────►  STM32 PA5   (SPI1 SCK)
ESP32 GPIO23  ──────────────►  STM32 PA7   (SPI1 MOSI)
```
Note: MISO (PA6) not connected — write-only SPI, no response needed.

### SPI — Validation Output
```
ESP32 GPIO22  ◄──────────────  STM32 PB5   (GPIO output)
```
STM32 drives PB5 HIGH on LED_ON command, LOW on LED_OFF. ESP32 reads GPIO22 to validate.

---

### PWM — Match
```
ESP32 GPIO32  ──────────────►  STM32 PB4   (GPIO ISR capture input)
ESP32 GPIO2   ◄──────────────  STM32 PB10  (TIM2 CH3 — reproduced PWM)
```
ESP32 generates a PWM signal. STM32 measures it on PB4 and reproduces the same frequency and duty cycle on PB10. ESP32 captures PB10 output and validates.

---

## Notes

- **Voltage levels**: Both boards run at 3.3V logic — no level shifting needed.
- **Common ground**: Always connect GND between boards first.
- **SPI MISO**: PA6 is not connected — SPI1 is write-only from master perspective.
- **SPI speed**: 500kHz — set in both ESP32 overlay and STM32 slave config.
- **UART baud**: 115200 on both sides.
- **PWM capture frequency range**: 20Hz–1500Hz reliable on ESP32 GPIO interrupt capture.
- **Debugger pins to avoid**: PA13 (SWDIO), PA14 (SWCLK), PB3 (SWO) — not used by DUT3.

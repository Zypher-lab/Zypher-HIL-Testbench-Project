# DUT3 Firmware — Logic Reference

STM32F401CC running Zephyr RTOS. Four independent modules run concurrently, each in its own thread or the main loop.

---

## Architecture

```
main()
├── switch_led_init()
├── fan_pwm_init()
├── uart_echo_init()
├── spi_cmd_init()
├── pwm_match_init()  → signals pwm_match_ready semaphore
│
├── [Thread] spi_cmd_thread     priority 2  stack 1024
├── [Thread] pwm_match_thread   priority 5  stack 2048
│
└── while(1) every 10ms
    ├── switch_led_update()
    └── fan_pwm_update()
```

---

## Modules

### switch_led — `src/switch_led.c`

Mirrors PB0 (input) to PB1 (output) every 10ms in the main loop.

```
PB0 HIGH  →  PB1 HIGH   (switch pressed → LED on)
PB0 LOW   →  PB1 LOW    (switch released → LED off)
```

- `gpio_pin_configure(GPIOB, 0, GPIO_INPUT | GPIO_PULL_DOWN)`
- `gpio_pin_configure(GPIOB, 1, GPIO_OUTPUT_INACTIVE)`
- Update: `gpio_pin_set_raw(gpio_dev, LED_PIN, gpio_pin_get_raw(gpio_dev, SWITCH_PIN))`

---

### fan_pwm — `src/fan_pwm.c`

Reads ADC on PA0 every 10ms and drives TIM1 CH1 PWM on PA8 proportionally.

```
Voltage on PA0  →  ADC raw (0–4095)  →  duty% = raw * 100 / 4095
PWM: 1kHz fixed frequency, duty = measured percentage
```

- ADC: `adc1` channel 0, 12-bit, internal reference, SYNC clock
- PWM: `timers1` prescaler 83, `pwm1` on PA8 (TIM1 CH1), channel 1
- `pwm_set(pwm_dev, 1, 1_000_000ns, pulse_ns, 0)`
- Print throttled to every 50 calls (~500ms)

---

### uart_echo — `src/uart_echo.c`

Interrupt-driven UART echo on USART2 (PA2 TX, PA3 RX).

```
Receives bytes → accumulates in rx_buf until '\r' or '\n'
→ sends the buffer back + CRLF
→ resets buffer
```

- `uart_irq_callback_set(uart_dev, uart_cb)`
- `uart_irq_rx_enable(uart_dev)`
- Echo via `uart_poll_out` per byte

---

### spi_cmd — `src/spi_cmd.c`

Zephyr SPI slave on SPI1 (PA4 NSS, PA5 SCK, PA7 MOSI). Receives one command byte per transaction and drives PB5.

```
0x01  →  PB5 HIGH   (LED_ON)
0x00  →  PB5 LOW    (LED_OFF)
```

- `spi_transceive(spi_dev, &slave_cfg, &tx_set, &rx_set)` — blocks until master clocks one byte
- Runs in dedicated thread priority 2 (higher than pwm_match)
- `slave_cfg.frequency = 500000` — required by STM32 driver even in slave mode

---

### pwm_match — `src/pwm_match.c`

Measures PWM on PB4 via GPIO interrupt (3-edge capture) and reproduces it on PB10 via TIM2 CH3.

```
PB4 (GPIO ISR)  →  measure period + pulse  →  PB10 (TIM2 CH3 PWM)
```

**Capture algorithm:**

```
1. Enable both-edge interrupt on PB4
2. ISR records edge_times[0,1,2] and edge_levels[0,1,2]
3. After 3 edges, give semaphore
4. period = edge_times[2] - edge_times[0]
5. if edge_levels[0]==1 (rising first):
       pulse = edge_times[1] - edge_times[0]
   else (falling first):
       pulse = edge_times[2] - edge_times[1]
6. freq_hz = cpu_hz / period
7. duty_pct = pulse * 100 / period
8. pwm_set(pwm_dev, 3, period_ns, pulse_ns, 0)
```

- Waits on `pwm_match_ready` semaphore before starting (ensures init completes first)
- 500ms timeout per capture attempt
- Valid frequency range: 20Hz–10kHz
- Thread priority 5, stack 2048

---

## Zephyr Configuration

**`prj.conf`**

```
CONFIG_GPIO=y
CONFIG_ADC=y
CONFIG_PWM=y
CONFIG_UART_INTERRUPT_DRIVEN=y
CONFIG_SERIAL=y
CONFIG_SPI=y
CONFIG_SPI_SLAVE=y
CONFIG_PRINTK=y
CONFIG_UART_CONSOLE=y
CONFIG_ADC_STM32=y
```

**Overlay peripherals**

| Peripheral | Zephyr Node | Pins       |
|------------|-------------|------------|
| USART2     | `usart2`    | PA2, PA3   |
| TIM1 PWM   | `pwm1`      | PA8        |
| TIM2 PWM   | `pwm2`      | PB10       |
| TIM3       | `timers3`   | (clock only)|
| ADC1       | `adc1`      | PA0        |
| SPI1       | board DTS   | PA4-PA7    |

---

## Build & Flash

```bash
source ~/zephyrproject/.venv/bin/activate
cd ~/zephyrproject

west build -b blackpill_f401cc \
    ~/projects/Arm_stm32/Zypher-HIL-Testbench-Project/firmware/dut_stm32_3 \
    -d ~/projects/Arm_stm32/Zypher-HIL-Testbench-Project/firmware/dut_stm32_3/build

west flash \
    -d ~/projects/Arm_stm32/Zypher-HIL-Testbench-Project/firmware/dut_stm32_3/build \
    --runner openocd
```

Boot output on `/dev/ttyACM0` at 115200:

```
*** Booting Zephyr OS ***
DUT3 starting
SPI1 slave ready
DUT3 ready
pwm_match thread started
```

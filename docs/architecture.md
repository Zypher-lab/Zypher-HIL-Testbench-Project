# Architecture

## ESP32 Generic Testbench Firmware

The ESP32 firmware is designed as a generic command-based testbench.
It does not contain hardcoded test cases. Instead, Robot Framework sends ZTB protocol commands over USB serial, and the ESP32 executes them using logical testbench resources.

## Static Architecture

| Module | Responsibility |
|---|---|
| `main.c` | Initializes modules, reads command lines, calls parser/executor, sends responses |
| `uart_transport.c` | Handles UART RX/TX line-based communication |
| `wifi_transport.c` | Handles WiFi TCP server RX/TX line-based communication |
| `ztb_protocol.c` | Parses ZTB command frames and formats responses |
| `test_executor.c` | Dispatches parsed commands to the correct service |
| `gpio_service.c` | Executes GPIO write/read/expect operations |
| `dac_service.c` | Executes DAC voltage output operations |
| `uart_service.c` | Executes UART write/read/send-expect operations toward DUT |
| `pwm_service.c` | Executes PWM output via LEDC and PWM capture via GPIO interrupt |
| `spi_service.c` | Executes SPI write and send-expect operations |
| `board_map.c` | Maps logical resources such as `DIO_OUT1` to ESP32 physical pins |

## Dynamic Architecture

```mermaid
sequenceDiagram
    participant Robot as Robot Framework
    participant UART as UART Transport
    participant Main as main.c
    participant Protocol as ztb_protocol
    participant Executor as test_executor
    participant GPIO as gpio_service
    participant BoardMap as board_map
    participant HW as ESP32 GPIO / STM32 DUT

    Robot->>UART: Send ZTB command line
    UART->>Main: uart_transport_read_line()
    Main->>Protocol: ztb_parse_frame(line)
    Protocol-->>Main: ztb_command_t

    Main->>Executor: test_executor_execute(command)
    Executor->>GPIO: gpio_service_write/read/expect()
    GPIO->>BoardMap: board_map_get_gpio_channel(ch)
    BoardMap-->>GPIO: physical GPIO pin + direction
    GPIO->>HW: Drive/read ESP32 GPIO pin
    HW-->>GPIO: actual signal value

    GPIO-->>Executor: operation result
    Executor-->>Main: ztb_response_t
    Main->>Protocol: ztb_format_response_with_cmd(response , cmd)
    Protocol-->>Main: response line
    Main->>UART: uart_transport_send_line(response)
    UART-->>Robot: ZTB response line
````
## WiFi Transport Architecture
```mermaid
sequenceDiagram
    participant Robot as Robot Framework
    participant TCP as WiFi TCP Transport
    participant Main as main.c
    participant Protocol as ztb_protocol
    participant Executor as test_executor
    participant Service as gpio/pwm/spi/uart/dac_service
    participant BoardMap as board_map
    participant HW as ESP32 GPIO / STM32 DUT

    Note over Robot,TCP: TCP Socket Port 5000 (No cable required)

    Robot->>TCP: TCP send ZTB frame
    TCP->>Main: wifi_transport_read_line()
    Main->>Protocol: ztb_parse_frame(line)
    Protocol-->>Main: ztb_command_t
    Main->>Executor: test_executor_execute(command)
    Executor->>Service: service_write/read/expect()
    Service->>BoardMap: board_map_get_channel(ch)
    BoardMap-->>Service: physical pin + direction
    Service->>HW: Drive/read ESP32 pin or DUT peripheral
    HW-->>Service: actual signal value
    Service-->>Executor: operation result
    Executor-->>Main: ztb_response_t
    Main->>Protocol: ztb_format_response_with_cmd(response , cmd)
    Protocol-->>Main: response line
    Main->>TCP: wifi_transport_send_line(response)
    TCP-->>Robot: TCP send ZTB response line
```

## Example GPIO_WRITE Flow

Command:

```text
ZTB|seq=1|cmd=GPIO_WRITE|ch=DIO_OUT1|val=1
```

Internal call flow:

```text
main.c
 -> uart_transport_read_line()
 -> ztb_parse_frame()
 -> test_executor_execute()
 -> gpio_service_write()
 -> board_map_get_gpio_channel()
 -> gpio_pin_set_raw()
 -> ztb_format_response()
 -> uart_transport_send_line()
```

Response:

```text
ZTB|seq=1|status=OK
```

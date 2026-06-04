# Architecture

## ESP32 Generic Testbench Firmware

The ESP32 firmware is designed as a generic command-based testbench.
It does not contain hardcoded test cases. Instead, Robot Framework sends ZTB protocol commands over USB serial, and the ESP32 executes them using logical testbench resources.

## Static Architecture

| Module | Responsibility |
|---|---|
| `main.c` | Initializes modules, reads command lines, calls parser/executor, sends responses |
| `uart_transport.c` | Handles UART RX/TX line-based communication |
| `ztb_protocol.c` | Parses ZTB command frames and formats responses |
| `test_executor.c` | Dispatches parsed commands to the correct service |
| `gpio_service.c` | Executes GPIO write/read/expect operations |
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
    Main->>Protocol: ztb_format_response(response)
    Protocol-->>Main: response line
    Main->>UART: uart_transport_send_line(response)
    UART-->>Robot: ZTB response line
````

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

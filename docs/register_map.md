# Modbus Register Map

## Serial communication configuration

The serial configuration occupies `0x1200-0x121A`. Each serial port owns a
`0x10`-register block.

| Port | Base address |
|---|---:|
| Serial port 0 | `0x1200` |
| Serial port 1 | `0x1210` |

| Offset | Field | Access | Encoding / valid range |
|---:|---|---|---|
| `0x00` | Baud code | R/W | `0=4800`, `1=9600`, `2=19200`, `3=38400`, `4=57600`, `5=115200` |
| `0x01` | Data bits | R/W | `7` or `8` |
| `0x02` | Parity | R/W | `0=None`, `1=Even`, `2=Odd` |
| `0x03` | Stop bits | R/W | `1` or `2` |
| `0x04` | Protocol | R/W | `0=Raw`, `1=Modbus RTU`, `2=Modbus ASCII` |
| `0x05` | Role | R/W | `0=Generic`, `1=Modbus Slave`, `2=Modbus Master` |
| `0x06` | Unit ID | R/W | `1-247` |
| `0x07` | Response timeout | R/W | `1-60000 ms` |
| `0x08` | Apply command | W | Write `0xA5A5` to request apply |
| `0x09` | Configuration status | R | `0=Active`, `1=Pending`, `2=Waiting TX`, `3=Applying`, `4=Error` |
| `0x0A` | Configuration revision | R | Increments after a successful apply |

Example: serial port 0 baud code is `0x1200`; serial port 1 baud code is
`0x1210`.

Writes update a pending configuration. They do not immediately reconfigure
the UART. The apply state machine and old-baud response timing are implemented
by the following integration step.

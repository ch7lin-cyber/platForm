# Communication architecture

## Serial data path

```text
L03 Modbus / SerialService
    -> HalSerial_WriteAsync()
    -> registered L01 write_async operation
    -> NXP USART/DMA driver
    -> UART hardware
```

Receive and completion events use the reverse notification path:

```text
NXP USART ISR/DMA callback
    -> HalSerial_NotifyReceiveFromIsr()
    -> L03 SerialService callback
    -> ring buffer and deferred event processing
```

L01 owns MCU instances, IRQ handlers, DMA handles, and vendor status conversion.
L02 owns stable serial types, driver registration, callbacks, and port dispatch.
L03 owns buffering, RTU/ASCII framing timing, Modbus behavior, diagnostics, and
communication policy.

For an MCU implementation that promises DMA transport, both receive and
transmit completion are produced by the L01 DMA/USART implementation. L02 and
L03 remain independent of the selected DMA controller and vendor SDK types.

## Port and board mapping

`HAL_SERIAL_PORT_0` and `HAL_SERIAL_PORT_1` identify stable platform ports. The
product BSP/configuration decides whether a port is used for RS485, debug, HMI,
or another board function. L03 must not contain names such as `FLEXCOMM3`,
`USART0`, GPIO pin numbers, or NXP SDK types.

A product may, for example, map port 0 to a boot/maintenance debug console and
external Modbus Slave, and port 1 to a Modbus Master. Once port 0 enters Modbus
operation, unsolicited console text must stop so it cannot corrupt protocol
frames. Whether the two ports share baud/format/protocol is also Product policy;
Platform provides the Pending/Apply mechanism without hard-coding that policy.

## Integration sequence

1. Wizard initializes pins, clocks, and configured peripherals.
2. Product-owned L01 creates its USART/DMA runtime contexts.
3. L01 calls `HalSerial_RegisterDriver()` for every available port.
4. Product configuration selects role, baud rate, parity, and stop bits.
5. L03 calls `SerialConfigurationApplyService_InitializePort()`. The apply
   service owns the callback registered with `SerialService` and forwards
   completed events to the configured consumer.
6. Runtime IRQ/DMA glue calls the `HalSerial_Notify*FromIsr()` entry points.
7. `SerialConfigurationApplyService_Process()` handles deferred events outside
   interrupt context.

## Runtime configuration apply sequence

```text
Modbus 0x06/0x10 writes Pending + Apply key
    -> Status = WAITING_TX_COMPLETE
    -> send Modbus response using old serial settings
    -> L01 TX/DMA completion notification
    -> SERIAL_SERVICE_EVENT_TX_COMPLETE
    -> ModbusRegisterAdapter_BeginApply()
    -> SerialService_ConfigurePort()
    -> HalSerial_Configure()
    -> L01 configure operation
    -> ModbusRegisterAdapter_CompleteApply()
```

The system tick must call `SerialConfigurationApplyService_Tick1ms()` instead
of calling `SerialService_Tick1ms()` separately. The apply service advances the
serial timing and cancels a waiting Apply if no response-complete event arrives
before its configured timeout.

Use `SerialConfigurationApplyService_WriteResponse()` for Modbus responses. A
non-retryable send failure or serial line error cancels the Apply request and
leaves the previous Active configuration unchanged. A Busy response may be
retried until the timeout expires.

Modbus broadcast requests do not have a response and therefore must not use
this response-complete apply path. A broadcast write to the Apply register
must be rejected by the future Modbus Slave dispatcher.

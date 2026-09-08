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

## Port and board mapping

`HAL_SERIAL_PORT_0` and `HAL_SERIAL_PORT_1` identify stable platform ports. The
product BSP/configuration decides whether a port is used for RS485, debug, HMI,
or another board function. L03 must not contain names such as `FLEXCOMM3`,
`USART0`, GPIO pin numbers, or NXP SDK types.

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

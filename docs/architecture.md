# System Platform architecture

## Dependency direction

```text
L05 Application                         product repository
    -> L04 Function Block               this repository
    -> L03 System Platform              this repository
    -> L02 BSP                          product repository
    -> L02 Platform HAL                 this repository
    -> L01 MCU Driver Adapter           product repository
    -> Vendor Wizard / SDK
    -> MCU hardware
```

`L02 BSP` and `L02 Platform HAL` share the same architectural level but have
different responsibilities. HAL is closer to L01 and describes generic hardware
capabilities. BSP is above HAL and assigns those capabilities to product/board
purposes.

## Repository ownership

| Component | Owner | Changes when MCU changes? |
|---|---|---|
| Vendor Wizard / SDK | product repository | Yes |
| L01 MCU Driver Adapter | product repository | Replace implementation |
| L02 Platform HAL API and core | `platForm` | No |
| L02 BSP mapping | product repository | Normally no; change only for a different board/product mapping |
| L03 System Platform | `platForm` | No |
| L04 Function Blocks | `platForm` | No |
| L05 Application | product repository | No |

## L01 to L02 contract

The reusable L02 HAL owns stable types and functions such as `HalSerial_*`.
L01 supplies the MCU-specific implementation by registering a driver operation
table during product startup:

```c
static const HalSerialDriverOps_t g_nxpUsartOps =
{
    NxpUsart_Initialize,
    NxpUsart_Configure,
    NxpUsart_WriteAsync,
    NxpUsart_AbortWrite,
    NxpUsart_IsWriteBusy
};

HalSerial_RegisterDriver(HAL_SERIAL_PORT_0,
                         &g_nxpUsartOps,
                         &g_nxpUsartContext);
```

The function names above are examples owned by the NXP L01 implementation; they
are deliberately absent from this repository.

Receive, transmit-complete, and error interrupts cross the boundary in the
opposite direction through `HalSerial_Notify*FromIsr()`.

## Startup composition

`main()` remains in the product repository and is the composition root:

```text
BOARD_InitBootPins/Clocks/Peripherals()
    -> ProductDriver_Init()
         -> HalSerial_RegisterDriver(...)
    -> PlatformHAL/ProductBSP initialization
    -> System Platform initialization
    -> Function Block initialization
    -> Application initialization
```

Wizard-generated pin, clock, and peripheral initialization is called exactly
once. L01 initializes only runtime handles, callbacks, DMA state, buffers, and
error state unless a peripheral was intentionally excluded from the Wizard.

## Boundary rules

1. Only product-owned L01 files may include MCU vendor headers.
2. L02 Platform HAL, L03 System Platform, and L04 Function Block must not include `fsl_*`, STM32 HAL,
   Renesas FSP, TI DriverLib, or other vendor headers.
3. L03 depends only on L02 interfaces; it must not call L01 directly.
4. L01 implements and registers the contracts declared by L02.
5. Product/board meanings such as RS485, heater PWM, sensor SPI, and status LED
   belong to BSP mapping, not to the generic HAL.
6. Dependency arrows always point downward; vendor code never calls L03–L05
   directly. ISR notifications enter through explicit L02 callback APIs.
7. L04 receives process data and services through L03 contracts. L04 never
   accesses L01, MCU instances, ISR glue, or vendor registers directly.

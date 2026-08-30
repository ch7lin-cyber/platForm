# L04 Function Block

This layer contains MCU-independent IEC 61131-3-style Function Blocks and their
shared lifecycle types. It may depend on L03 services and data contracts, but it
must never include L01 or MCU vendor headers.

## Categories

- `Control`: PID, on/off, fuzzy, feedforward, gain scheduling, Smith predictor,
  process estimator, and auto-tuning blocks.
- `Alarm`: limit, deviation, standby/hold, latch, and alarm aggregation blocks.
- `Safety`: sensor-break, over-temperature, interlock, safe-output, and shutdown
  decision blocks.
- `Event`: event detection, debounce, latch, acknowledgement, and routing blocks.
- `Utility`: filters, scaling, validation, timing, and reusable application
  utility blocks.

`FunctionBlockLifecycle` supplies an optional edge-triggered lifecycle for
command-style blocks. Cyclic control blocks may use their own deterministic
20 ms execution contract instead.

## Dependency rule

```text
L05 Application
    -> L04 Function Block
    -> L03 System Platform
    -> L02 BSP / Platform HAL
```

L04 must receive hardware values through L03 service interfaces or explicit
input structures. It must not call `HalSerial`, NXP SDK functions, or hardware
registers directly.

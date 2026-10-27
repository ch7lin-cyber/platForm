# L04 Function Block

2026-10-27：完成狀態統一區分「L04 實作／封裝、Host 單元測試／mock 測試、產品串接、MCU 實測」。[狀態定義與各 FB 總表](VERIFICATION_STATUS.md)；不再將 Host 驗證視為獨立於單元測試的第三項完成工作。


This layer contains MCU-independent IEC 61131-3-style Function Blocks and their
shared lifecycle types. It may depend on L03 services and data contracts, but it
must never include L01 or MCU vendor headers.

## Categories

各已實作 FB 的 SVG 方塊圖、輸入／輸出引腳與參數文件，請參閱 [FBD 索引](FBD_INDEX.md)。各模組 README 的最開頭直接嵌入同一份 SVG；輸入／輸出折線以兩個公開入口分別繪圖。

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

## Planned standalone blocks

See [implementation checklist](IMPLEMENTATION_CHECKLIST.md) for the control,
utility, diagnostics, and recording scaffolds. Blocks marked unimplemented in the checklist remain excluded from builds and have no upstream/downstream wiring.

## C／C++ 共用介面規則

含函式宣告的公開 `.h` 使用 `#ifdef __cplusplus` 包覆 `extern "C"` 頭尾宣告；include 放在連結宣告之外。對外型別與函式介面保持 C／C++ 相容。`.c` 以 C 編譯並包含自己的 Header，不需另外加 `extern "C"`。預留 FB 空架構亦遵守此規則；只有巨集、型別或 include 的 Header 不需要 C linkage。

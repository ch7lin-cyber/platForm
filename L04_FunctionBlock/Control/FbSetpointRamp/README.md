![FbSetpointRamp FBD：輸入、輸出、功能與參數設定](FbSetpointRamp_BlockDiagram.svg)

# FbSetpointRamp

## 摘要

**完成狀態（2026-10-27 用語統一）：L04 實作完成；Host 單元測試通過。**

| 追蹤項目 | 狀態 |
|---|---|
| 產品串接 | 尚未完成；詳細待辦見本模組文件與產品整合文件 |
| MCU 實測 | 尚未實測 |

狀態用語見 [完成狀態與驗證定義](../../VERIFICATION_STATUS.md)；[測試報告](_UnitTest/TestResult/FbSetpointRamp_HostTest.md)。本次為文件整理，不代表重新執行測試。

已實作的 L04 功能方塊；本頁依公開 Header 補齊 SVG FBD 與介面導覽。圖示本身不代表產品串接或 MCU 實測完成。

Setpoint rise / fall ramp。

測試摘要與細則：[FbSetpointRamp_HostTest.md](_UnitTest/TestResult/FbSetpointRamp_HostTest.md)。

## 介面與參數

### 輸入：FbSetpointRampInput

| 欄位 | C 型別 |
|---|---|
| `target` | `float` |
| `dt_seconds` | `float` |
| `enable` | `bool` |

### 輸出：FbSetpointRampOutput

| 欄位 | C 型別 |
|---|---|
| `value` | `float` |
| `reached` | `bool` |
| `valid` | `bool` |
| `status` | `FbSetpointRampStatus` |

### 設定：FbSetpointRampConfig

| 欄位 | C 型別 |
|---|---|
| `rise_rate` | `float` |
| `fall_rate` | `float` |

完整數值範圍、單位、初始化、禁能及錯誤行為請參閱 [公開 Header](../FbSetpointRamp.h) 與上述測試細則；呼叫端單位 U 不預設為溫度或 0.001°C。

## SVG FBD 維護說明

2026-10-26：方塊圖已依 [公開 Header](../FbSetpointRamp.h) 核對。左側為輸入、右側為輸出，外框內為功能分組與參數設定；Init／Reset 的資源與初值另列，不代表新增執行引腳。標示 API result 的列是函式回傳值，非 Output 結構成員。內部箭頭表示功能關係，精確優先序與錯誤行為以程式及本文契約為準。圖面使用英文介面名稱，說明沿用本頁繁體中文文件。

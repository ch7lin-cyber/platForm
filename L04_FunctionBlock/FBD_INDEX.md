# L04 Function Block SVG FBD 索引

## 摘要

2026-10-27：完成狀態統一區分「L04 實作／封裝、Host 單元測試／mock 測試、產品串接、MCU 實測」。[狀態定義與各 FB 總表](VERIFICATION_STATUS.md)；不再將 Host 驗證視為獨立於單元測試的第三項完成工作。


2026-10-26：25 個已實作模組，共 26 張公開入口 FBD（輸入／輸出折線分開繪製）。PID 採先前確認的 SVG 範例，其餘使用相同的外框、左輸入、右輸出、虛線功能分組與參數設定格式。各模組 README 與原有測試報告共用同一份 SVG，避免維護多份圖。

FBD 為目前介面的文件化，不代表正式 PID callback、Register、實體輸出或 MCU 實測已完成。沒有 ENABLE／RESET 的既有介面不會因換圖而新增引腳；禁能語意也不統一改寫。

| FB／入口 | SVG FBD | 功能與參數文件 |
|---|---|---|
| FbAlarm | [方塊圖](Alarm/FbAlarm/FbAlarm_BlockDiagram.svg) | [README](Alarm/FbAlarm/README.md) |
| FbMultiChannelAlarm | [方塊圖](Alarm/FbMultiChannelAlarm/FbMultiChannelAlarm_BlockDiagram.svg) | [README](Alarm/FbMultiChannelAlarm/README.md) |
| FbCascadeCoordinator | [方塊圖](Control/FbCascadeCoordinator/FbCascadeCoordinator_BlockDiagram.svg) | [README](Control/FbCascadeCoordinator/README.md) |
| FbFeedforward | [方塊圖](Control/FbFeedforward/FbFeedforward_BlockDiagram.svg) | [README](Control/FbFeedforward/README.md) |
| FbOutputLimiter | [方塊圖](Control/FbOutputLimiter/FbOutputLimiter_BlockDiagram.svg) | [README](Control/FbOutputLimiter/README.md) |
| FbPid | [方塊圖](Control/FbPid/FbPid_BlockDiagram.svg) | [README](Control/FbPid/README.md) |
| FbPidGroupSelector | [方塊圖](Control/FbPidGroupSelector/FbPidGroupSelector_BlockDiagram.svg) | [README](Control/FbPidGroupSelector/README.md) |
| FbPowerVoltageCompensation | [方塊圖](Control/FbPowerVoltageCompensation/FbPowerVoltageCompensation_BlockDiagram.svg) | [README](Control/FbPowerVoltageCompensation/README.md) |
| FbProfileSequencer | [方塊圖](Control/FbProfileSequencer/FbProfileSequencer_BlockDiagram.svg) | [README](Control/FbProfileSequencer/README.md) |
| FbSetpointRamp | [方塊圖](Control/FbSetpointRamp/FbSetpointRamp_BlockDiagram.svg) | [README](Control/FbSetpointRamp/README.md) |
| FbBatchMetrics | [方塊圖](Diagnostics/FbBatchMetrics/FbBatchMetrics_BlockDiagram.svg) | [README](Diagnostics/FbBatchMetrics/README.md) |
| FbHeaterResistance | [方塊圖](Diagnostics/FbHeaterResistance/FbHeaterResistance_BlockDiagram.svg) | [README](Diagnostics/FbHeaterResistance/README.md) |
| FbProcessResponse | [方塊圖](Diagnostics/FbProcessResponse/FbProcessResponse_BlockDiagram.svg) | [README](Diagnostics/FbProcessResponse/README.md) |
| FbRecordTrigger | [方塊圖](Recording/FbRecordTrigger/FbRecordTrigger_BlockDiagram.svg) | [README](Recording/FbRecordTrigger/README.md) |
| FbDeadTime | [方塊圖](Utility/FbDeadTime/FbDeadTime_BlockDiagram.svg) | [README](Utility/FbDeadTime/README.md) |
| FbDivide | [方塊圖](Utility/FbDivide/FbDivide_BlockDiagram.svg) | [README](Utility/FbDivide/README.md) |
| FbInBandTimer | [方塊圖](Utility/FbInBandTimer/FbInBandTimer_BlockDiagram.svg) | [README](Utility/FbInBandTimer/README.md) |
| FbInputSwitchOver | [方塊圖](Utility/FbInputSwitchOver/FbInputSwitchOver_BlockDiagram.svg) | [README](Utility/FbInputSwitchOver/README.md) |
| FbLeadLag | [方塊圖](Utility/FbLeadLag/FbLeadLag_BlockDiagram.svg) | [README](Utility/FbLeadLag/README.md) |
| FbLinearization | [方塊圖](Utility/FbLinearization/FbLinearization_BlockDiagram.svg) | [README](Utility/FbLinearization/README.md) |
| FbLogic | [方塊圖](Utility/FbLogic/FbLogic_BlockDiagram.svg) | [README](Utility/FbLogic/README.md) |
| FbMovingAverage | [方塊圖](Utility/FbMovingAverage/FbMovingAverage_BlockDiagram.svg) | [README](Utility/FbMovingAverage/README.md) |
| FbSelector | [方塊圖](Utility/FbSelector/FbSelector_BlockDiagram.svg) | [README](Utility/FbSelector/README.md) |
| FbInputLinearization | [方塊圖](Utility/FbSignalLinearization/FbInputLinearization_BlockDiagram.svg) | [README](Utility/FbSignalLinearization/README.md) |
| FbOutputLinearization | [方塊圖](Utility/FbSignalLinearization/FbOutputLinearization_BlockDiagram.svg) | [README](Utility/FbSignalLinearization/README.md) |
| FbWeightedSum | [方塊圖](Utility/FbWeightedSum/FbWeightedSum_BlockDiagram.svg) | [README](Utility/FbWeightedSum/README.md) |

## 尚未實作

[FbC7gLogic](Utility/FbC7gLogic/README.md) 目前只有規劃，沒有正式 Header／實作，故不繪製帶有確定引腳的正式 FBD。SYSTEM 的 Event Service 與設定事件消費者亦不列為獨立數學 FB。

## 圖面規則

- 外框對應單一運算實例或無狀態呼叫，不代表整個 L05 通道。MultiChannelAlarm 是一個跨來源比較實例。
- 資料陣列以陣列引腳表示；每個 Input／Output 成員均可在圖中對應。複合參數的內部欄位列於設定區。
- 左／右實線為訊號或結果關係，設定區使用虛線；State／Lifecycle 說明歷史與呼叫端資源。
- SVG 內的箭頭不代表所有條件分支的精確執行順序；單位、合法範圍與清除規則以各 README 及 Header 為準。
- 溫度專用 FB 使用既有 0.001°C 契約；通用浮點 FB 的單位由呼叫端決定；PID 後端暫轉 0.1°C 的註記保留。

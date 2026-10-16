# L04 Function Block 建立與完成清單

參考 C7G／C7S 功能需求建立的獨立方塊規劃，不代表其專用演算法。

- [x] 建立分類目錄及 `.c／.h` 空架構。
- [x] 每個佔位檔標示「尚未完成」。
- [x] 建立功能與驗證進度清單。
- [ ] 定義各方塊的正式介面。
- [ ] 完成功能實作與單元驗證。

目前狀態以表格及各方塊完成條件為準；空架構建立不等於功能完成。FbLogic、FbAlarm 已有實作與單元測試；FbDivide、FbWeightedSum、FbSelector、FbMovingAverage 均已實作並加入單元測試。FbSetpointRamp、FbOutputLimiter、FbInBandTimer、FbLinearization 已完成實作與 Host 單元測試。FbDeadTime、FbLeadLag 已完成明確契約與 Host 單元測試（不代表產品專用演算法）。其餘標示尚未完成的方塊仍是佔位。
尚未完成的佔位方塊不加入 CMake／MCU 專案編譯，也不串接前後層；已完成且具單元測試的 FB 納入 CMake Host 驗證。

## 方塊清單

第一批：基本控制、程序與批次統計。第二批：進階運算、補償與診斷。第三批：串級協調。
PID 實作前先檢查既有模組，避免重複開發。

| 批次 | 方塊 | 功能 | 狀態 |
|---|---|---|---|
| 1 | [FbSetpointRamp](Control/FbSetpointRamp.h) | 設定值升降斜率 | 已完成（Host 驗證） |
| 1 | [FbProfileSequencer](Control/FbProfileSequencer.h) | 分段程序控制 | 尚未完成 |
| 1 | [FbInBandTimer](Utility/FbInBandTimer.h) | 保證浸泡／區間內計時 | 已完成（Host 驗證） |
| 1 | [FbPid](Control/FbPid.h) | PID 控制介面預留；實作前確認既有 PID 是否可沿用 | 尚未完成 |
| 1 | [FbOutputLimiter](Control/FbOutputLimiter.h) | 輸出限幅及變化率限制 | 已完成（Host 驗證） |
| 1 | [FbWeightedSum](Utility/FbWeightedSum.h) | 加權加總 | 已完成（實作與單元測試） |
| 1 | [FbDivide](Utility/FbDivide.h) | 除法及除零檢查 | 已完成（實作與單元測試） |
| 1 | [FbSelector](Utility/FbSelector.h) | 訊號選擇及最大／最小值 | 已完成（實作與單元測試） |
| 2 | [FbMovingAverage](Utility/FbMovingAverage.h) | 移動平均 | 已完成（host 驗證） |
| 2 | [FbLinearization](Utility/FbLinearization.h) | 分段線性轉換 | 已完成（Host 驗證） |
| 2 | [FbLeadLag](Utility/FbLeadLag.h) | 超前／落後處理 | 已完成（Host 驗證） |
| 2 | [FbDeadTime](Utility/FbDeadTime.h) | 純延遲 | 已完成（Host 驗證） |
| 2 | [FbPowerVoltageCompensation](Control/FbPowerVoltageCompensation.h) | 電源電壓補償 | 尚未完成 |
| 2 | [FbHeaterResistance](Diagnostics/FbHeaterResistance.h) | 加熱器電阻估算 | 尚未完成 |
| 1 | [FbBatchMetrics](Diagnostics/FbBatchMetrics.h) | 批次製程統計 | 尚未完成 |
| 2 | [FbProcessResponse](Diagnostics/FbProcessResponse.h) | 製程響應評估 | 尚未完成 |
| 2 | [FbRecordTrigger](Recording/FbRecordTrigger.h) | 資料記錄觸發判定 | 尚未完成 |
| 3 | [FbCascadeCoordinator](Control/FbCascadeCoordinator.h) | 串級協調介面預留 | 尚未完成 |

## 各方塊完成條件

只有正式介面、實作、單元驗證及文件全部完成後，才能勾選「已完成」。

### FbSetpointRamp

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbProfileSequencer

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbInBandTimer

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

- Host 測試報告：[FbInBandTimer_HostTest.md](Utility/FbInBandTimer/_UnitTest/TestResult/FbInBandTimer_HostTest.md)。

### FbPid

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbOutputLimiter

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbWeightedSum

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbDivide

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbSelector

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbMovingAverage

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

### FbLinearization

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

- Host 測試報告：[FbLinearization_HostTest.md](Utility/FbLinearization/_UnitTest/TestResult/FbLinearization_HostTest.md)。

### FbLeadLag

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

- Host 測試報告：[FbLeadLag_HostTest.md](Utility/FbLeadLag/_UnitTest/TestResult/FbLeadLag_HostTest.md)。

### FbDeadTime

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] 單元驗證通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] 已完成。

- Host 測試報告：[FbDeadTime_HostTest.md](Utility/FbDeadTime/_UnitTest/TestResult/FbDeadTime_HostTest.md)。

### FbPowerVoltageCompensation

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbHeaterResistance

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbBatchMetrics

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbProcessResponse

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbRecordTrigger

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

### FbCascadeCoordinator

- [ ] 正式介面與單位／範圍定義完成。
- [ ] 功能及異常輸入處理完成。
- [ ] 單元驗證通過並留下測試報告。
- [ ] 文件、記憶體需求及重設行為確認。
- [ ] 已完成。

## 共用設計約束

- Config、Input、State、Output 分離；每個實例各自保存狀態。
- 時間步長由呼叫端提供，定義單位、零值及過大時間步長的處理。
- 不使用動態配置；緩衝區由呼叫端提供並明確定義容量。
- 方塊不直接操作 ADC、GPIO、通訊、Register Map 或 NVM。
- 保留必要的有效性與錯誤狀態；詳細 DEBUG 記錄可編譯關閉。
- 程序定義由外部提供唯讀資料；段數上限不直接照搬產品規格。
- 電阻估算檢查電流過小與取樣時間對齊；健康分析需有效激勵及基準。
- 串級方塊目前僅佔位，不建立 PID 間的實際串接。

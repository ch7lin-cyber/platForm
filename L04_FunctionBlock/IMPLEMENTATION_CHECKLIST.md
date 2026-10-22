# L04 Function Block 建立與完成清單

參考 C7G／C7S 功能需求建立的獨立方塊規劃，不代表其專用演算法。

2026-10-22 盤點：20 個具有 .c／.h 的運算 FB 中，12 項已實作並有 Host 測試、1 項 FbPid 已有 CKLIB 封裝與 mock 驗證但實體驗證待完成、7 項仍為佔位；另有僅文件規劃的 FbC7gLogic。完整 DTDM04 缺項與產品分層對照記錄於主專案 _Product/Docs/FB_DTDM04_GAP_REVIEW.md。**正式 PID callback、Register、實體輸出與 MCU 實測仍待接入。**

- [x] 建立分類目錄及 `.c／.h` 空架構。
- [x] 每個佔位檔標示「尚未完成」。
- [x] 建立功能與驗證進度清單。
- [ ] 定義各方塊的正式介面。
- [ ] 完成功能實作與單元驗證。

目前狀態以表格及各方塊完成條件為準；空架構建立不等於功能完成。FbLogic、FbAlarm 已有實作與單元測試，並支援 ENABLE 停用時輸出 0 及 DISABLED 狀態；FbDivide、FbWeightedSum、FbSelector、FbMovingAverage 均已實作並加入單元測試。FbSetpointRamp、FbOutputLimiter、FbInBandTimer、FbLinearization 已完成實作與 Host 單元測試。FbDeadTime、FbLeadLag 已完成明確契約與 Host 單元測試（不代表產品專用演算法）。其餘標示尚未完成的方塊仍是佔位。
尚未完成的佔位方塊不加入 CMake／MCU 專案編譯，也不串接前後層；已完成且具單元測試的 FB 納入 CMake Host 驗證。

## 方塊清單

**FbAlarm：單一警報已實作並通過 13 組 Host 驗證（2026-10-22）。** 包含八種比較、0.001°C 刻度、可調待機帶（預設 ±0.100°C）、0～100 秒成立延遲、模式 0 強制 OFF、AUTO／MANUAL 解除及獨立峰值清除。MANUAL 僅在條件正常時接受 Reset，控制 STOP 不清除鎖存；這些行為為專案契約，與 DTDM04 原 HOLD 語意不同。產品 branch2_ver1 已建立 L05 Alarm／Logic adapter 並通過 Host 整合驗證；尚未完成設備對照、MCU 實測、Alarm Register 與 NVM 整合。詳見 [FbAlarm 規格與限制](Alarm/FbAlarm/README.md)。

**FbLogic：單一 L04 實例契約已完成。** 2026-10-22 已補齊 OFF 強制零的優先規則、預設設定、設定錯誤分類、ON／OFF 計時方向及高位元忽略規則；2048 組真值與 9 組時間／狀態測試通過。OFF＋反向優先序為專案契約，不代表設備相容或 MCU 實測完成。L05 來源選擇、多組配置及排程不列入 L04 完成條件。詳見 [FbLogic 手冊功能對照](Utility/FbLogic/MANUAL_COMPLIANCE.md)。

**FbC7gLogic：獨立規劃，尚未實作。** 2026-10-21 決定將 C7G 邏輯功能與 FbLogic 分開；C7G 功能不再列為 FbLogic 缺項。規格與完成條件見 [FbC7gLogic 規劃](Utility/FbC7gLogic/README.md)。

第一批：基本控制、程序與批次統計。第二批：進階運算、補償與診斷。第三批：串級協調。
PID 實作前先檢查既有模組，避免重複開發。

| 批次 | 方塊 | 功能 | 狀態 |
|---|---|---|---|
| 1 | [FbSetpointRamp](Control/FbSetpointRamp.h) | 設定值升降斜率 | 已完成（Host 驗證） |
| 1 | [FbProfileSequencer](Control/FbProfileSequencer.h) | 分段程序控制 | 尚未完成 |
| 1 | [FbInBandTimer](Utility/FbInBandTimer.h) | 保證浸泡／區間內計時 | 已完成（Host 驗證） |
| 1 | [FbPid](Control/FbPid.h) | L04 CKLIB 封裝：AUTO_TUNE／ON_OFF／PID／PID_FB | 封裝及 mock 驗證已做；實體庫與正式整合待完成 |
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
| 1 | [FbAlarm](Alarm/FbAlarm.h) | 單一警報、延遲、待機、手動鎖存與峰值 | 已實作（Host 驗證）；與 DTDM04 有明訂差異 |
| 1 | [FbLogic](Utility/FbLogic.h) | 四輸入邏輯、反向與 ON／OFF 延遲 | 已實作（Host 驗證）；OFF 優先規則為專案契約 |
| 獨立 | [FbC7gLogic](Utility/FbC7gLogic/README.md) | C7G 邏輯 | 僅文件規劃，尚未實作；不列入 DTDM04 缺項 |

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

- [x] 已建立正式 L04 封裝介面與單位定義（PV／SV 目前為 0.1°C）。
- [x] 四種 CKLIB 模式、生命週期及錯誤狀態封裝已實作。
- [x] 產品 adapter 的 CKLIB mock Host 測試已建立。
- [ ] 實體 CKLIB 數值行為、backend 資源及 MCU 建置／實測驗證。
- [ ] 正式 PID callback、參數組管理與產品輸出鏈整合完成。
- [ ] 完整完成；不得以 mock 測試宣稱實體控制效果已驗證。

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

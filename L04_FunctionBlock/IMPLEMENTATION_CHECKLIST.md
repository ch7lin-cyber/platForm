# L04 Function Block 建立與完成清單

參考 C7G／C7S 功能需求建立的獨立方塊規劃，不代表其專用演算法。

2026-10-25 更新：25個具有.c／.h的運算模組中，24個已有專案核心與Host測試，1個FbPid為CKLIB封裝／mock驗證；原七個佔位均已補核心。FbSignalLinearization含輸入／輸出兩入口，統計為一個共用模組；另有僅文件規劃的FbC7gLogic。完整產品串接／設備相容性不是L04核心完成的同義詞。**正式PID callback、Register／NVM、實體輸出與MCU實測仍待接入。** 本輪以Git提交保存核心與驗證文件，產品整合狀態仍依下列未完成項目判定。

## 狀態用語（2026-10-27）

實作與測試分開判定；「Host」是執行環境，不是與單元測試並列的另一套完成項目。統一寫為「L04 實作完成；Host 單元測試通過」，PID 則為「L04 封裝完成；Host mock 單元測試通過」。詳見 [狀態定義與逐項總表](VERIFICATION_STATUS.md)。

目前 25 個已實作模組的正式介面已建立；FbC7gLogic 仍只有規劃文件。下列完成條件限於 L04 公開契約，產品串接、實體 CKLIB、Register／NVM／輸出與 MCU 實測分開追蹤，不沿用早期「其餘皆為佔位」的描述。本次僅整理文件，未重新執行歷史測試。

## 方塊清單

新增 FbFeedforward：事件前饋曲線、三階段、Force與取消／重觸發生命週期已通過Host／ASan／UBSan。SYSTEM／L05、PID加總、anti-windup及Register／NVM／MCU未接。詳見 [規格與方塊圖](Control/FbFeedforward/README.md)。

新增 FbPidGroupSelector：四組候選參數包裝、全組驗證、固定／依SV選組及外部鎖組，Host／ASan／UBSan通過；後端整組套用、AT結果／NVM與MCU未完成。詳見 [規格與方塊圖](Control/FbPidGroupSelector/README.md)。不以選組成功當作PID已實際套用。

新增 FbMultiChannelAlarm：可變來源極值溫差、成立延遲、Hold／Reset、單次動作請求及品質失效去重已通過 Host／ASan／UBSan。L05／SYSTEM 命令／Register／NVM／MCU 未整合。詳見 [規格與方塊圖](Alarm/FbMultiChannelAlarm/README.md)。此為原盤點後新增項目。

新增 FbInputSwitchOver：單實例雙來源切換／平均、故障策略、有效性與 ENABLE／Reset 已通過 Host／ASan／UBSan 驗證；L05／Register／NVM／MCU 未整合。詳見 [規格與方塊圖](Utility/FbInputSwitchOver/README.md)。此為原日期盤點後新增項目。

新增折線包裝：FbSignalLinearization 提供單訊號輸入／輸出兩入口，已完成 BYPASS／VALUE／DEVIATION、OFF／內插／左端步階、Offset 與最終限制，Host 測試通過；以共用核心加包裝計，不更改下列原盤點日期的數量。產品 L05／Register／NVM／MCU 仍未整合。規格及方塊圖見 [折線包裝](Utility/FbSignalLinearization/README.md)。

**FbAlarm：單一警報已實作並通過 13 組 Host 驗證（2026-10-22）。** 包含八種比較、0.001°C 刻度、可調待機帶（預設 ±0.100°C）、0～100 秒成立延遲、模式 0 強制 OFF、AUTO／MANUAL 解除及獨立峰值清除。MANUAL 僅在條件正常時接受 Reset，控制 STOP 不清除鎖存；這些行為為專案契約，與 DTDM04 原 HOLD 語意不同。產品 branch2_ver1 已建立 L05 Alarm／Logic adapter 並通過 Host 整合驗證；尚未完成設備對照、MCU 實測、Alarm Register 與 NVM 整合。詳見 [FbAlarm 規格與限制](Alarm/FbAlarm/README.md)。

**FbLogic：單一 L04 實例契約已完成。** 2026-10-22 已補齊 OFF 強制零的優先規則、預設設定、設定錯誤分類、ON／OFF 計時方向及高位元忽略規則；2048 組真值與 9 組時間／狀態測試通過。OFF＋反向優先序為專案契約，不代表設備相容或 MCU 實測完成。L05 來源選擇、多組配置及排程不列入 L04 完成條件。詳見 [FbLogic 手冊功能對照](Utility/FbLogic/MANUAL_COMPLIANCE.md)。

**FbC7gLogic：獨立規劃，尚未實作。** 2026-10-21 決定將 C7G 邏輯功能與 FbLogic 分開；C7G 功能不再列為 FbLogic 缺項。規格與完成條件見 [FbC7gLogic 規劃](Utility/FbC7gLogic/README.md)。

第一批：基本控制、程序與批次統計。第二批：進階運算、補償與診斷。第三批：串級協調。
PID 實作前先檢查既有模組，避免重複開發。

| 批次 | 方塊 | 功能 | 狀態 |
|---|---|---|---|
| 1 | [FbSetpointRamp](Control/FbSetpointRamp.h) | 設定值升降斜率 | L04 實作完成；Host 單元測試通過 |
| 1 | [FbProfileSequencer](Control/FbProfileSequencer.h) | 分段程序控制 | L04 實作完成；Host 單元測試通過；持久化及產品串接待接 |
| 1 | [FbInBandTimer](Utility/FbInBandTimer.h) | 保證浸泡／區間內計時 | L04 實作完成；Host 單元測試通過 |
| 1 | [FbPid](Control/FbPid.h) | L04 CKLIB 封裝：AUTO_TUNE／ON_OFF／PID／PID_FB | L04 封裝完成；Host mock 單元測試通過；實體庫與正式整合待完成 |
| 1 | [FbOutputLimiter](Control/FbOutputLimiter.h) | 輸出限幅及變化率限制 | L04 實作完成；Host 單元測試通過 |
| 1 | [FbWeightedSum](Utility/FbWeightedSum.h) | 加權加總 | L04 實作完成；Host 單元測試通過 |
| 1 | [FbDivide](Utility/FbDivide.h) | 除法及除零檢查 | L04 實作完成；Host 單元測試通過 |
| 1 | [FbSelector](Utility/FbSelector.h) | 訊號選擇及最大／最小值 | L04 實作完成；Host 單元測試通過 |
| 2 | [FbMovingAverage](Utility/FbMovingAverage.h) | 移動平均 | L04 實作完成；Host 單元測試通過 |
| 2 | [FbLinearization](Utility/FbLinearization.h) | 分段線性轉換 | L04 實作完成；Host 單元測試通過 |
| 2 | [FbLeadLag](Utility/FbLeadLag.h) | 超前／落後處理 | L04 實作完成；Host 單元測試通過 |
| 2 | [FbDeadTime](Utility/FbDeadTime.h) | 純延遲 | L04 實作完成；Host 單元測試通過 |
| 2 | [FbPowerVoltageCompensation](Control/FbPowerVoltageCompensation.h) | 電源電壓補償 | L04 實作完成；Host 單元測試通過；產品輸出鏈待接 |
| 2 | [FbHeaterResistance](Diagnostics/FbHeaterResistance.h) | 加熱器電阻估算 | L04 實作完成；Host 單元測試通過；RMS來源與產品處置待接 |
| 1 | [FbBatchMetrics](Diagnostics/FbBatchMetrics.h) | 批次製程統計 | L04 實作完成；Host 單元測試通過；產品整合未完成 |
| 2 | [FbProcessResponse](Diagnostics/FbProcessResponse.h) | 已知步階響應觀測 | L04 實作完成；Host 單元測試通過；產品實驗串接待接 |
| 2 | [FbRecordTrigger](Recording/FbRecordTrigger.h) | 資料記錄觸發判定 | L04 實作完成；Host 單元測試通過；產品記錄服務未接 |
| 3 | [FbCascadeCoordinator](Control/FbCascadeCoordinator.h) | 串級SV換算 | L04 實作完成；Host 單元測試通過；雙PID協調待接 |
| 1 | [FbAlarm](Alarm/FbAlarm.h) | 單一警報、延遲、待機、手動鎖存與峰值 | L04 實作完成；Host 單元測試通過；與 DTDM04 有明訂差異 |
| 1 | [FbLogic](Utility/FbLogic.h) | 四輸入邏輯、反向與 ON／OFF 延遲 | L04 實作完成；Host 單元測試通過；OFF 優先規則為專案契約 |
| 增補 | [FbSignalLinearization](Utility/FbSignalLinearization/README.md) | 專案公開契約見 README | L04 實作完成；Host 單元測試通過；產品串接與 MCU 另列 |
| 增補 | [FbInputSwitchOver](Utility/FbInputSwitchOver/README.md) | 專案公開契約見 README | L04 實作完成；Host 單元測試通過；產品串接與 MCU 另列 |
| 增補 | [FbMultiChannelAlarm](Alarm/FbMultiChannelAlarm/README.md) | 專案公開契約見 README | L04 實作完成；Host 單元測試通過；產品串接與 MCU 另列 |
| 增補 | [FbPidGroupSelector](Control/FbPidGroupSelector/README.md) | 專案公開契約見 README | L04 實作完成；Host 單元測試通過；產品串接與 MCU 另列 |
| 增補 | [FbFeedforward](Control/FbFeedforward/README.md) | 專案公開契約見 README | L04 實作完成；Host 單元測試通過；產品串接與 MCU 另列 |
| 獨立 | [FbC7gLogic](Utility/FbC7gLogic/README.md) | C7G 邏輯 | 尚未實作；尚無單元測試紀錄 |

## 各方塊完成條件

只有正式介面、實作、Host 單元測試通過紀錄及文件齊備，才標示「L04 實作完成；Host 單元測試通過」；此標記不包含產品串接與 MCU 實測。PID 封裝採 mock 標記，不能列為實體 CKLIB 驗證完成。

### FbSetpointRamp

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbProfileSequencer

已完成程序核心、循環／連接、命令及等待／逾時，Host／ASan／UBSan通過。正式規格見 [方塊圖與契約](Control/FbProfileSequencer/README.md)。

- [x] 正式介面與單位／範圍定義完成。
- [x] L04核心功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、借用記憶體與重設行為確認。
- [x] L04專案契約完成；不宣稱完整DTDM04相容。
- [ ] L05／Register／NVM斷電接續與MCU整合完成。

### FbInBandTimer

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

- Host 測試報告：[FbInBandTimer_HostTest.md](Utility/FbInBandTimer/_UnitTest/TestResult/FbInBandTimer_HostTest.md)。

### FbPid

- [x] L04公開PV／SV為0.001°C，CKLIB邊界暫轉0.1°C；下一版Colin統一後端後須移除轉換，見 [PID單位待辦](Control/FbPid/README.md)。
- [x] 四種 CKLIB 模式、生命週期及錯誤狀態封裝已實作。
- [x] 產品 adapter 的 CKLIB mock Host 測試已建立。
- [ ] 實體 CKLIB 數值行為、backend 資源及 MCU 建置／實測驗證。
- [ ] 正式 PID callback、參數組管理與產品輸出鏈整合完成。
- [ ] 完整完成；不得以 mock 測試宣稱實體控制效果已驗證。

### FbOutputLimiter

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbWeightedSum

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbDivide

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbSelector

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbMovingAverage

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

### FbLinearization

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

- Host 測試報告：[FbLinearization_HostTest.md](Utility/FbLinearization/_UnitTest/TestResult/FbLinearization_HostTest.md)。

### FbLeadLag

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

- Host 測試報告：[FbLeadLag_HostTest.md](Utility/FbLeadLag/_UnitTest/TestResult/FbLeadLag_HostTest.md)。

### FbDeadTime

- [x] 正式介面與單位／範圍定義完成。
- [x] 功能及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認。
- [x] L04 實作完成；Host 單元測試通過。

- Host 測試報告：[FbDeadTime_HostTest.md](Utility/FbDeadTime/_UnitTest/TestResult/FbDeadTime_HostTest.md)。

### FbPowerVoltageCompensation

- [x] 正式介面與單位／範圍定義完成。
- [x] 平方補償、旁路及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 無狀態契約及文件完成；見 [規格與方塊圖](Control/FbPowerVoltageCompensation/README.md)。
- [x] L04專案補償契約完成。
- [ ] L05／輸出鏈／anti-windup、Register／NVM及MCU完成。

### FbHeaterResistance

- [x] 正式介面與單位／範圍定義完成。
- [x] 比值估算與品質／時間檢查完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 無狀態契約及文件完成；見 [規格與方塊圖](Diagnostics/FbHeaterResistance/README.md)。
- [x] L04專案診斷契約完成。
- [ ] L05／RMS來源／產品處置與MCU整合完成。

### FbBatchMetrics

- [x] 正式介面與單位／範圍定義完成。
- [x] 批次統計核心及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認；見 [規格與方塊圖](Diagnostics/FbBatchMetrics/README.md)。
- [x] L04專案契約完成。
- [ ] L05／資料保存／UI及MCU整合完成。

### FbProcessResponse

- [x] 正式介面與單位／範圍定義完成。
- [x] 已知基準觀測及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認；見 [規格與方塊圖](Diagnostics/FbProcessResponse/README.md)。
- [x] L04專案觀測契約完成，不宣稱模型自動辨識或PID整定。
- [ ] L05／實驗品質與結果保存／UI、MCU整合完成。

### FbRecordTrigger

- [x] 正式介面與單位／範圍定義完成。
- [x] 週期／事件觸發及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認；見 [規格與方塊圖](Recording/FbRecordTrigger/README.md)。
- [x] L04專案觸發契約完成。
- [ ] L05／實際記錄服務與保存回報、Register／NVM／MCU完成。

### FbCascadeCoordinator

- [x] 正式介面與單位／範圍定義完成。
- [x] MV→SV換算核心及異常輸入處理完成。
- [x] Host 單元測試通過並留下測試報告。
- [x] 文件、記憶體需求及重設行為確認；見 [規格與方塊圖](Control/FbCascadeCoordinator/README.md)。
- [x] L04專案換算契約完成，非完整DTDM04相容結論。
- [ ] 雙PID／L05、AT／校準、anti-windup與保護協調、Register／NVM／MCU完成。

## 共用設計約束

- Config、Input、State、Output 分離；每個實例各自保存狀態。
- 時間步長由呼叫端提供，定義單位、零值及過大時間步長的處理。
- 不使用動態配置；緩衝區由呼叫端提供並明確定義容量。
- 方塊不直接操作 ADC、GPIO、通訊、Register Map 或 NVM。
- 保留必要的有效性與錯誤狀態；詳細 DEBUG 記錄可編譯關閉。
- 程序定義由外部提供唯讀資料；段數上限不直接照搬產品規格。
- 電阻估算檢查電流過小與取樣時間對齊；健康分析需有效激勵及基準。
- 串級 SV 換算核心已實作並有 Host 單元測試通過紀錄；雙 PID 的正式協調與產品串接尚未完成。

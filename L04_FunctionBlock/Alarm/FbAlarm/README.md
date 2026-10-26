![FbAlarm FBD：輸入、輸出、功能與參數設定](FbAlarm_BlockDiagram.svg)

# FbAlarm：單通道警報

## 摘要

2026-10-22：已完成八種比較模式、0.001°C 刻度、可調待機帶、成立延遲、AUTO／MANUAL 解除模式、獨立警報重置與峰值清除、反向及有效性檢查。[Host 測試報告](_UnitTest/TestResult/FbAlarm_HostTest.md)記錄 13 組測試通過，含 ASan／UBSan。

比較模式及成立延遲參考 DTDM04 操作手冊 v1.5（2026-06-16）第 4.2.1～4.2.2 節，頁 4-9～4-10。AUTO／MANUAL、Reset 接受條件與峰值清除為本專案契約，不能宣稱與設備完全相同。尚未完成 MCU 實測、Alarm Register、NVM 及實體輸出整合。產品 branch2_ver1 已建立 L05 Alarm／Logic adapter 與 Host 整合驗證，詳見產品 _Product/Docs/L05_ALARM_LOGIC.md。C7G EVENT 不在本 FB 範圍。

## 刻度與比較模式

PV、SV、H／L、待機帶及峰值均使用 int32_t，每單位為 0.001°C；25.123°C 表示為 25123。H／L 範圍 −200000～3000000，即 −200～3000°C。相對門檻以 int64_t 加減，避免溢位。時間獨立使用毫秒。

| 模式 | 成立條件 | 解除／維持 |
|---|---|---|
| 0 無警報 | 永不成立 | 最終輸出強制 OFF，跳過反向。 |
| 1 相對上下限 | PV > SV＋H 或 PV < SV－L | 回到範圍內比較結果為 false。 |
| 2 相對上限 | PV > SV＋H | PV ≤ SV＋H。 |
| 3 相對下限 | PV < SV－L | PV ≥ SV－L。 |
| 4 絕對上下限 | PV > H 或 PV < L | 回到範圍內比較結果為 false。 |
| 5 絕對上限 | PV > H | PV ≤ H。 |
| 6 絕對下限 | PV < L | PV ≥ L。 |
| 7 遲滯上限 | PV > SV＋H | PV < SV＋L；等號維持前次狀態。 |
| 8 遲滯下限 | PV < SV－H | PV > SV－L；等號維持前次狀態。 |

絕對上下限與遲滯模式要求 L≤H；相對上下限要求 H＋L≥0。這些一致性檢查屬專案契約。非溫度來源須由外層明確定義刻度，不能直接混入。

## 設定與引腳

| 結構 | 欄位 | 定義 |
|---|---|---|
| FbAlarmConfig_t | mode | 0～8 比較模式。 |
| 設定 | reset_mode | AUTO=0（預設）、MANUAL=1；未知值拒絕。取代舊 HOLD bit。 |
| 設定 | alarm_high、alarm_low | H／L，0.001°C。相對模式可使用合法的有號偏移量。 |
| 設定 | standby_band | 0～3000000，預設 100＝0.100°C；STANDBY 關閉時忽略此欄位及其範圍驗證。 |
| 設定 | delay_ms | 成立延遲 0～100000 ms；沒有解除延遲。 |
| 設定 | options | bit0 待機、bit1 反向、bit3 峰值；bit2 已移除，傳入會回報設定錯誤。 |
| FbAlarmInput_t | process_value、setpoint | PV／SV，0.001°C。 |
| 輸入 | elapsed_ms | 距前次呼叫實際經過時間，由 L05 提供。 |
| 輸入 | enable、valid | 停用或資料無效時立即強制 OFF 並清除執行狀態，峰值保留。 |
| 輸入 | reset | 上升緣提出警報重置；不兼作執行狀態初始化或峰值清除。 |
| 輸入 | clear_peak | 上升緣獨立清除峰值，不解除警報。 |
| FbAlarmOutput_t | alarm_active | 最終邏輯輸出，已套用 MANUAL 鎖存與反向，不等於實體接點。 |
| 輸出 | condition_active | 待機允許後的比較／遲滯結果。 |
| 輸出 | qualified_active | 成立延遲完成後的結果，在鎖存及反向之前。 |
| 輸出 | alarm_latched | MANUAL 模式的警報鎖存，取代 hold_latched。 |
| 輸出 | standby_ready | 已解除待機。 |
| 輸出 | delay_active、delay_elapsed_ms | 正在等待成立與已累積時間。 |
| 輸出 | peak_valid、peak_high、peak_low | 是否有有效峰值及最高／最低值，刻度 0.001°C；無記錄時數值為 0。 |
| 輸出 | status | OK、DISABLED、INVALID_ARGUMENT、INVALID_CONFIGURATION、INVALID_INPUT、RESET、RESET_REJECTED。 |

## 執行與解除規則

處理順序：待機 → 比較／遲滯 → 成立延遲 → AUTO／MANUAL 鎖存 → Reset 判定 → 峰值 → 反向。

1. **待機：**啟用 STANDBY 時，PV 須先進入 SV±standby_band（含邊界）。設為 0 須 PV==SV，不代表停用待機；不隨 UI 顯示位數改變。解除後離開範圍不會重新待機。
2. **成立延遲：**首次觀察條件成立由 0 ms 起算，後續連續成立才累積 elapsed_ms。條件消失立即歸零；0 延遲立即成立。累積到期後飽和於設定值，避免超大 elapsed_ms 溢位。呼叫間未取樣到的短暫變化無法辨識。
3. **AUTO：**不鎖存；條件恢復（含遲滯解除）即解除，沒有 OFF 延遲。
4. **MANUAL：**qualified_active=true 才鎖存；比較條件恢復後仍保留，等待新 Reset 命令。
5. **Reset：**以當次 condition_active 判定，不以 qualified_active 判定。條件仍成立，即使還在成立延遲期間，也回報 RESET_REJECTED；不清除鎖存、遲滯、待機或計時，計時仍正常推進。條件為 false 時回報 RESET 並清除鎖存，保留待機及峰值；不強制最終輸出 OFF，反向仍照常處理。在 AUTO 也遵循相同命令判定，未鎖存時可接受為無操作。
6. **命令邊緣：**Reset 被拒絕後，持續為 true 不會在恢復時自動重試，須先回 false 再給新上升緣。Reset 持續為 true 不能阻擋新的警報。clear_peak 同樣採上升緣。
7. **控制 STOP：**不再是本 FB 的輸入，不能清除或抑制警報鎖存。L05 不應把一般控制 STOP 轉成 enable=false；若外層明確停用 FB，仍適用強制 OFF／清除執行狀態規則。
8. **峰值：**只有 PEAK 啟用且 qualified_active=true 才捕捉 PV，跨警報累積。成立延遲前、純鎖存及反向造成的正常 ON 不捕捉。Reset 不清峰值；clear_peak 不清鎖存。清除當次若 qualified_active=true，以當次 PV 重建；否則 peak_valid=false。兩命令可同時發出，Reset 被拒絕也不影響峰值清除。
9. **反向：**最後才反轉。一般待機／延遲或已接受 Reset 後，最終輸出可能因反向而為 true；應查看 condition_active、qualified_active、alarm_latched 分辨狀態。停用、無效輸入／設定及模式 0 優先強制 OFF。
10. **設定變更：**mode／reset_mode／H／L／standby_band／delay／options 改變時清除執行狀態與峰值，新設定當次即重新運算。待機帶變更重新待機，但若 PV 已在新帶內可當次解除。設定變更本身可清除鎖存，外層應透過明確 Apply 執行。
11. **動態 SV：**不自動清除待機、峰值或鎖存；以當次 SV 比較。Reset 已不具備重新初始化功能；需要明確重新初始化時使用 FbAlarm_Initialize()。
12. **錯誤與停用：**清除執行狀態但保留峰值。命令邊緣在停用期間仍追蹤，clear_peak 仍有效；Reset 不在恢復時補執行。NULL input／output 清除執行狀態與邊緣歷史。狀態與引數不可重疊，同一實例需循序呼叫，初始化不可在每週期重複執行。

## 與 DTDM04 的差異

DTDM04 的保持 Bit2 說明停止控制可清除；本專案改為獨立 reset_mode，MANUAL 停止控制仍鎖存。Reset 僅在條件正常時接受、峰值獨立清除及清除後重建政策，均為專案明確定義，手冊未完整交代這些時序。可調待機帶與 0.001°C 精度也是專案擴充。成立延遲維持手冊方向，不增加解除延遲。

## 初始化與遷移

```c
FbAlarm_t alarm;
FbAlarmConfig_t config;
FbAlarmInput_t input = {0};
FbAlarmOutput_t output;

FbAlarm_Initialize(&alarm);
FbAlarm_DefaultConfig(&config); /* AUTO、待機帶 0.100°C、模式 OFF */
config.mode = FB_ALARM_MODE_RELATIVE_HIGH;
config.reset_mode = FB_ALARM_RESET_MANUAL;
config.alarm_high = 10000; /* SV + 10.000°C */
config.delay_ms = 2000U;
config.options = FB_ALARM_OPTION_PEAK;
input.enable = true;
input.valid = true;
input.setpoint = 100000;
input.process_value = 112000;
input.elapsed_ms = 20U;
(void)FbAlarm_Execute(&alarm, &config, &input, &output);
```

舊 HOLD bit 應明確轉成 reset_mode=MANUAL，再移除 options bit2；不可直接帶入舊 options。control_running 已移除，hold_latched 改名為 alarm_latched。既有狀態須重新初始化。僅使用零設定會得到 AUTO、零待機帶；需要預設 0.100°C 時呼叫 FbAlarm_DefaultConfig()。

舊 0.1°C PV／SV／H／L 整數值乘以 100 才是新刻度；溫度 Sensor Snapshot engineering_value 已為 0.001°C，不可再乘。來源過期或轉換失敗應設 valid=false。

## Register 介接契約（尚未接入產品）

每個警報實例規劃獨立 reset_mode（U16：0=AUTO、1=MANUAL）及 standby_band（Float32 實際 °C，兩個 word，預設 0.100）。待機帶先拒絕非有限值、負值及超出產品／FB 範圍的值，再乘以 1000、四捨五入；讀回已量化值。UI 位數不得改寫門檻。

設定使用 Pending／Apply；Float32 以 FC10 成對寫入，拒絕 FC06 半值更新。實際產品可縮小待機帶允許範圍。

每通道三組警報，規劃兩個獨立命令 Register：

| 命令 | bit0 | bit1 | bit2 |
|---|---|---|---|
| Alarm Reset | Alarm1 Reset | Alarm2 Reset | Alarm3 Reset |
| Alarm Peak Clear | Alarm1 Peak Clear | Alarm2 Peak Clear | Alarm3 Peak Clear |

命令寫入即提出、不需要 Apply，讀回 0；未知 bit 拒絕。每次有效寫入都是新命令，L05 須將各 bit 轉成獨立單次脈波，連續命令也須確保 L04 觀察到兩次上升緣。命令接受不等同警報一定清除，執行結果須由 status／診斷回報。

產品 L05 adapter 已提供單組命令及狀態查詢；尚無 Alarm Register 群組，上述位址、Modbus 命令解碼與 NVM 未實作。FB 不直接讀 Register、不發布 SYSTEM 事件，也不操作 GPIO。最終輸出須經產品／Safety／輸出服務許可。

## SVG FBD 維護說明

2026-10-26：方塊圖已依 [公開 Header](../FbAlarm.h) 核對。左側為輸入、右側為輸出，外框內為功能分組與參數設定；Init／Reset 的資源與初值另列，不代表新增執行引腳。標示 API result 的列是函式回傳值，非 Output 結構成員。內部箭頭表示功能關係，精確優先序與錯誤行為以程式及本文契約為準。圖面使用英文介面名稱，說明沿用本頁繁體中文文件。

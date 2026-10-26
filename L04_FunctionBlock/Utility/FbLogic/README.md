![FbLogic FBD：輸入、輸出、功能與參數設定](FbLogic_BlockDiagram.svg)

# FbLogic：單一邏輯運算實例

## 摘要

2026-10-22：已完成單一 L04 邏輯實例的四輸入運算、各輸入反向、結果反向、ON／OFF 延遲、ENABLE、RESET、預設設定與診斷。2048 組真值及 9 組時間／狀態測試通過，另通過 ASan／UBSan。

一個 FbLogic_t 代表一組邏輯運算，不固定產品通道數或組數。每個實例獨立保存輸出及時間，不使用 heap、MCU Timer、GPIO 或狀態資料庫。L05 依產品需求建立實例、選擇來源、提供週期時間並分派結果。

以 DTDM04 v1.5 第 5.2.4 節（頁 5-25～5-26）的單組功能為參考；OFF 優先於反向是本專案明確定義，手冊未明訂此組合。不宣稱完整設備時序相容。C7G 功能由另一個 FB 分開處理。

## 輸入、輸出與參數

| 類別 | 欄位 | 契約 |
|---|---|---|
| 設定 | operator_type | OFF=0、AND=1、OR=2、XOR=3；其他值回報 INVALID_CONFIGURATION。 |
| 設定 | input_inversions[4] | 各取 bit0，先反向再運算；高位元忽略，也不觸發設定變更。 |
| 設定 | output_inverted | 布林結果反向後，才套用延遲。 |
| 設定 | on_delay_ms、off_delay_ms | uint32_t 毫秒，0～UINT32_MAX；0 立即切換。 |
| 輸入 | inputs[4] | A／B／C／D 各取 bit0；不修改呼叫端資料。 |
| 輸入 | enable | false 優先強制輸出 0，回報 DISABLED，清除輸出與計時。 |
| 輸入 | reset | 上升緣清除輸出與計時，當次輸出 0、回報 RESET。持續 true 不重複執行。 |
| 輸入 | elapsed_ms | L05 提供距前次呼叫的實際時間，不是預設週期。 |
| 輸出 | value、raw_value | 最終輸出／延遲前反向結果，皆為 0 或 1。 |
| 輸出 | delay_active | 是否正在等待 ON 或 OFF 切換。 |
| 輸出 | on_delay_active、off_delay_active | 分別代表等待 0→1／1→0，不會同時成立。 |
| 輸出 | delay_elapsed_ms | 目前累積等待時間；取消、到期、停用、重設或錯誤時歸零。 |
| 輸出 | status | OK、DISABLED、INVALID_ARGUMENT、INVALID_CONFIGURATION、RESET。 |

AND 使用全部四個輸入；未使用的輸入應由 L05 設為運算後的中性值：AND 為 1，OR／XOR 為 0。若該輸入啟用反向，來源常數也須相應調整。XOR 為四輸入奇偶運算，奇數個 1 才輸出 1。

## 時序與狀態細則

1. 優先序：NULL 參數檢查 → ENABLE → 設定檢查 → OFF → 設定變更 → RESET → 運算與延遲。
2. OFF 立即強制 value／raw_value=0、清除計時並回報 DISABLED，不受反向或 OFF 延遲影響；舊版 OFF＋反向可能為 1 的行為已取消。
3. 首次觀察到目標值與目前輸出不同，計時由 0 開始，不回算當次 elapsed_ms。之後連續符合才累積，到期或超過門檻即切換。
4. 等待期間目標值回到目前輸出，立即取消計時；再次改變從 0 重啟。計時比較剩餘時間再累加，避免 UINT32_MAX 溢位。
5. 停用、OFF、錯誤後恢復時，輸出由 0 開始重新判斷。非零 ON 延遲重新計時；零延遲可立即切換。
6. 運算、有效反向 bit、結果反向或延遲設定改變，清除輸出及計時後以新設定重新判斷。輸入反向的高位元改變不會重啟計時。
7. Reset 邊緣在所有非 NULL input／output／instance 呼叫中追蹤，包括停用、OFF 或設定錯誤。已觀察為 true 的命令不會在恢復後再執行；NULL 引數錯誤則重新初始化執行狀態及邊緣歷史。
8. enable=false 時忽略設定，即使 config=NULL 或運算值錯誤仍回報 DISABLED；enable=true 的 config=NULL 則回報 INVALID_ARGUMENT。
9. FB 只能觀察呼叫當次的值，無法偵測兩次呼叫之間未取樣到的變化。實際切換時間受 L05 呼叫週期與排程延遲影響。
10. 狀態與 config／input／output 儲存區不得重疊；同一實例須循序呼叫。不同實例可以使用各自配置或共用唯讀設定。

## 呼叫範例

```c
FbLogic_t logic;
FbLogicConfig_t config;
FbLogicInput_t input = {0};
FbLogicOutput_t output;

FbLogic_Initialize(&logic); /* 只初始化一次 */
FbLogic_DefaultConfig(&config); /* OFF、無反向、零延遲 */
config.operator_type = FB_LOGIC_OPERATOR_OR;
config.on_delay_ms = 100U;
config.off_delay_ms = 200U;
input.enable = true;
input.inputs[0] = 1U;
input.elapsed_ms = 20U; /* 實際呼叫間隔；每次更新 */
(void)FbLogic_Execute(&logic, &config, &input, &output);
```

例如 ON 延遲 100 ms：首次看到 1 記為 t=0，後續 t=20／40／60／80 ms 保持 0，t=100 ms 切為 1。

## L05 與產品整合邊界

狀態資料庫來源、固定 0／1、實例數、結果發布、DO 分派、Register、NVM 及 PID 前後排程由外層負責，不列為單一 L04 缺項。手冊的 16 組配置不應寫死在本 FB。若採用手冊時間格式（0.1 秒），外層先以足夠位寬乘以 100 並驗證範圍，再傳入毫秒。

產品 branch2_ver1 已建立 L05 Logic adapter 與 Host 整合驗證，來源含常數、Alarm、Logic 及 DI，詳見產品 _Product/Docs/L05_ALARM_LOGIC.md；MCU 驗證尚未完成。輸入來源失效時應由 L05 明確停用 FB，避免沿用過期來源值。完整細則見[功能對照](MANUAL_COMPLIANCE.md)與[測試報告](_UnitTest/TestResult/FbLogic_HostTest.md)。

## SVG FBD 維護說明

2026-10-26：方塊圖已依 [公開 Header](../FbLogic.h) 核對。左側為輸入、右側為輸出，外框內為功能分組與參數設定；Init／Reset 的資源與初值另列，不代表新增執行引腳。標示 API result 的列是函式回傳值，非 Output 結構成員。內部箭頭表示功能關係，精確優先序與錯誤行為以程式及本文契約為準。圖面使用英文介面名稱，說明沿用本頁繁體中文文件。

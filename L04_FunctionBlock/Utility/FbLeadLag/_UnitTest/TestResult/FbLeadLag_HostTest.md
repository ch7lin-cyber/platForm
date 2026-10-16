# 超前／落後補償功能方塊：Host 驗證報告

```text
 Config                                  Output
 lead_seconds >= 0 ─────┐             ┌── value [U]
 lag_seconds > 0 ───────┤             ├── valid
                       ▼             └── status
 Input           ┌───────────────────────┐
 value [U] ─────►│       FbLeadLag       │
 dt_seconds ────►│ backward Euler        ├────►
 sample_valid ──►│ H=(1+Tlead*s)/(1+Tlag*s)│
                 └──────────┬────────────┘
                            ↕ State（每實例獨立）
                   double lag_value、float value
                   active_lead/lag、initialized
 Init / Reset(config, state, initial_value) ──► 平衡初始狀態
```

## 介面與功能摘要

此為明確定義的可重用數位補償契約，不宣稱等同 C7G／C7S 專用演算法。純運算，不接硬體、L03/L05、Register Map 或 NVM。

- Config：lead_seconds 有限且非負；lag_seconds 有限且嚴格大於零，單位秒。
- Input：value 有限、單位由呼叫端統一；dt_seconds 有限且嚴格大於零；sample_valid 表示這次有可接受的新樣本。
- State：double lag_value 保留未捨入的低通狀態；value 為最後 float 輸出；active 時常數綁定目前 epoch。
- Output：value、valid、status。正常啟動即有有效輸出。NO_SAMPLE 或任何錯誤的 valid=false，value 只是有限的前次值診斷；非法狀態輸出零，不可拿無效輸出控制設備。
- Init/Reset(config,state,initial_value)：以同一初值設定低通狀態與輸出。初值等於恆定輸入時無人工啟動突波。真實階躍在 lead>lag 時可產生超越量，這不是安全限幅器。
- 每次 Execute 先驗指標、狀態、Config，再判 sample_valid，最後驗數值。NO_SAMPLE 忽略 value/dt、不前進也不補算遺失時間。呼叫端自行定義下次接受樣本代表的時間間隔。
- 允許逐次改變正 dt，零 dt、負 dt、NaN、Inf 皆拒絕。變更任一時常數必須先 Reset，包含不變參數的 Reset 也會重新開始。

## 離散化與數值策略

對 H(s)=(1+Tlead*s)/(1+Tlag*s) 採後向 Euler，數學等價式如下，其中 q 是低通狀態：

```text
d = Tlag + dt
q_new = q_old + dt/d * (u - q_old)
y     = q_new + Tlead/d * (u - q_old)
固定 dt 的極點 = Tlag/(Tlag+dt)，介於 0 與 1。
```

程式不直接照抄可能相消的算式。小權重用增量形式保留細小變化；大權重分別計算 dt/d 與 Tlag/d，以保留 alpha 捨入為 1 時的衰減尾端。lead<lag 的輸出也採穩定凸組合，將純捨入越界夾回端點；lead>lag 用 u+(lead-lag)/d*(u-q_old)。lead=lag 特別直通，lead=0 特別回傳低通。q 與候選 y 完成驗證才一起提交，超出 float 範圍回 NUMERIC_ERROR，不飽和、不污染狀態。

支援 binary32 float、至少 binary64 double；編譯時檢查範圍與精度。要求 round-to-nearest、漸進下溢、不開 FTZ/DAZ。拒絕 __FAST_MATH__ 與 __FINITE_MATH_ONLY__；執行期浮點控制模式無法由編譯檢查得知。double 可累積小於 float ULP 的變化，但仍可能捨去小於 double 解析度的增量。本模組無 libm 呼叫依賴；測試的 nextafterf/fabsl 等需要 -lm。

## 逐項測試與結果

| 編號 | 輸入／操作 | 預期與實際結果 |
|---|---|---|
| UT-01 | 未初始化；以 8 啟動、輸入 8 | 拒絕未初始化；平衡輸出 8，通過 |
| UT-02 | lead=0、lag=1、dt=1，0→8 | 4、6、7，通過 |
| UT-03 | 接前例 dt 改 3 | 7.75，通過 |
| UT-04 | lead=3、lag=1，0→8；重設為 8 | 16、12、10；平衡 8，通過 |
| UT-05 | lead=lag=FLT_MAX，極小 dt、相反極值 | 完全直通 FLT_MAX 與 1，通過 |
| UT-06 | sample_valid=false 且數值 NaN | NO_SAMPLE，狀態逐位不變，通過 |
| UT-07 | 改 Config、同／不同 Config 重設 | 未重設拒絕；重設後第一步 4，通過 |
| UT-08 | value/dt 的 NaN、±Inf，dt=0/-1 | 全部拒絕且狀態不變，通過 |
| UT-09 | 不合法 Config／初值／Init／Reset | 原狀態逐位不變，通過 |
| UT-10 | 全部空指標與可觀察狀態毀損 | 正確錯誤碼、有限診斷、不寫入狀態，通過 |
| UT-11 | 極大 lead、極小 lag/dt、相反極值 | 溢位原子拒絕，後續平衡仍可執行，通過 |
| UT-12 | q=FLT_MAX，u=0，dt=FLT_MAX，極小 lag | 保留 FLT_MIN 及最小 subnormal 尾端，通過 |
| UT-13 | 1→2，每次 dt=1e-9，共 1000 次 | 輸出超過 1；內部值約 1.000001，通過 |
| UT-14 | -FLT_MAX→FLT_MAX，低通權重 1/2 | 有限 0，不溢位，通過 |
| UT-15 | 兩個實例不同歷史 | 分別輸出 4 與 14，通過 |
| UT-16 | 3 組時常數，各 10000 個變 dt/輸入樣本 | 對照 long double 參考，誤差≤2e-6×max(1,abs(y))，通過 |
| UT-17 | 接近 identity 的 lead<lag、極大 dt | 保留最小 subnormal 輸出尾端，通過 |
| UT-18 | 獨立凸權重捨入、±FLT_MAX 平衡 | 不誤報溢位，兩端恆定輸出完全保持，通過 |

UT-18 為獨立審查發現後加入的回歸案例，修正輸出凸組合的捨入越界。測試程式：Tests/Unit/L04_FunctionBlock/test_fb_lead_lag.c。

## 記憶體、生命週期與限制

無 heap、遞迴或外部緩衝區。Host ABI 實測 Config/Input/State/Output 分別為 8/12/24/12 bytes；不同 ABI 應以 sizeof 為準。每次執行 O(1)。State 首次須全零初始化，之後只能透過 API 更新；不同實例不共用狀態。Reset 不推進時間，失敗不修改狀態。參數、狀態、輸出必須是互不重疊且型別正確的儲存空間，禁止並行寫入。檢查可偵測的非法數值，不保證偵測所有記憶體毀損。

## 重現

從 repository 根目錄執行（GCC 14.2.0，Linux x86-64）：

```sh
for opt in 0 2; do
 gcc -std=c99 -pedantic -Wall -Wextra -Werror -O$opt \
  -I L04_FunctionBlock/Utility L04_FunctionBlock/Utility/FbLeadLag.c \
  Tests/Unit/L04_FunctionBlock/test_fb_lead_lag.c -lm -o /tmp/test_lead_lag
 /tmp/test_lead_lag
done
gcc -std=c99 -pedantic -Wall -Wextra -Werror -O1 \
 -fsanitize=address,undefined -fno-omit-frame-pointer \
 -I L04_FunctionBlock/Utility L04_FunctionBlock/Utility/FbLeadLag.c \
 Tests/Unit/L04_FunctionBlock/test_fb_lead_lag.c -lm -o /tmp/test_lead_lag_san
ASAN_OPTIONS=detect_leaks=0 /tmp/test_lead_lag_san
```

O0/O2 與 AddressSanitizer+UndefinedBehaviorSanitizer 通過。容器的 ptrace 限制使 LeakSanitizer 不可執行，因此明確關閉 leak 檢查；不得解讀為 leak sanitizer 通過。C99 O0 編譯 production objects，與 C++11 caller 混合連結、不加 -lm 通過。-ffast-math 與 -ffinite-math-only 編譯拒絕檢查通過。既有十個 FB 與本次兩個 FB 的 O0/O2 共十二套單元測試通過；這不是全平台測試。

本次未執行 MCU、硬體在迴路或閉迴路整合；CMake/CTest 因環境無 cmake 指令未執行，僅更新目標。未驗證所有編譯器與 MCU FPU 設定，移植前須確認 double 能力與浮點模式。

## 獨立審查補充探測

另以非交付測試程式進行 3,000,000 組隨機時常數／恆定極值探測，以及 980,517 組有效隨機位元輸入的一步響應，均通過。後者以獨立 long double 加權參考檢查，容許誤差為 2e-6 相對誤差加兩個 float 最小次正規 ULP，並核對真正超出 float 範圍時的拒絕行為。這些是額外審查，不併入正式 18 個案例及其中 30,000 個序列樣本。

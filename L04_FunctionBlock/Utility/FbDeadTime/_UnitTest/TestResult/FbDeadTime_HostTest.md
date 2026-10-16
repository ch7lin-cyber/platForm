# 純延遲功能方塊 FbDeadTime：Host 單元測試

```text
 Config: delay_samples N ----+
                            v
 Input: value, sample_valid -> [ FbDeadTime_Execute ] -> Output:
                            ^   |                       value, valid,
                            |   v                       ready, status
                   caller-owned State
                   buffer, capacity, N,
                   write_index, filled,
                   initial_value, last_value,
                   initialized
                            ^
 Init / Reset(config, state, buffer, capacity, initial_value)
                            |
                 caller-owned float buffer[N]
```

## 介面與契約

- 本版為通用離散純延遲，不主張複製 C7G／C7S 專有演算法。Config.delay_samples 為 size_t 整數 N，單位是「接受的有效樣本數」，不是秒。沒有時間戳、時鐘或 dt；遺漏樣本不補插。以固定週期呼叫且樣本皆有效時，才可由外部將 N 換算為時間。
- Input.value 為有限 float，單位由呼叫端決定。sample_valid=false 代表這次沒有新樣本，忽略 value（包含 NaN／±Inf），回傳 NO_SAMPLE，完全保留 State 與 buffer，不讀取 ring 內容；Output.valid=false、ready=false，value 保留最後有限診斷值。
- Init／Reset 接收 config、state、float buffer、capacity 與有限 initial_value。呼叫端先將 State 零初始化，再呼叫 Init。N>0 必須提供容量至少 N 的有效 float 陣列；N=0 可用 NULL、capacity=0，也接受普通非 NULL 緩衝區。NULL 配非零容量拒絕。N 和 capacity 不得超過 SIZE_MAX/sizeof(float)，避免不可能的物件長度。
- 首 N 次接受樣本輸出 initial_value，valid=true、ready=false；第 N+1 次輸出第 1 次接受的樣本，ready=true。N=0 第一次有效呼叫即直通，valid=true、ready=true。ready 描述本次輸出是否來自真實樣本，不是單純 ring 已填滿；第 N 次仍為 false。所有錯誤及 NO_SAMPLE 均清除當次 valid／ready，不鎖存旗標。
- Config.N 在初始化期間保持不變；Execute 比對已保存的 N，任何改動均回傳 INVALID_CONFIGURATION，連 NO_SAMPLE 也不能繞過。恢復原 N 可繼續原歷史；改用新 N 必須明確 Reset／Init，重新建立取樣期間。
- 優先序：必要指標 → State 結構 → Config.N 一致性 → sample_valid → 輸入有限性 → 即將讀出的 ring 樣本有限性 → 更新。若 output 可寫，錯誤回傳 status 與 Output.status 一致，valid／ready=false；結構有效則 value 是最後輸出，結構失效則為零。數值錯誤不被當成新樣本。
- State 驗證 initialized、buffer/capacity/N、filled≤N、索引範圍、initial_value 與 last_value 有限性；啟動未填滿時驗證 index=filled 且 last_value=initial_value；N=0 要求 index=filled=0。達穩態僅驗證本次讀取位置的 ring 值，不做 O(N) 掃描。壞值出現在其他位置會到該位置被讀取時才發現。
- 所有 Init／Reset 失敗與 Execute 失敗／NO_SAMPLE 保持 State 與 buffer 位元組不變。Output 診斷更新不包含在此原子性保證。延遲樣本為 NaN／±Inf 時先拒絕，不覆寫該槽位，也不推進索引。
- 介面資料須為互不重疊、正確型別的有效儲存體，不支援 State、Input、Config、Output 或 buffer 別名、並行讀寫。State 與 buffer 由呼叫端提供，但使用期間欄位及活動槽位由方塊維護。C 無法證明指標生命期、實際配置容量或任意記憶體破壞；有效範圍內的竄改不一定可偵測。

## 記憶體、重設與數值

- 每實例所需 ring 為 N*sizeof(float) bytes，可提供更多容量，僅操作前 N 格；不配置 heap、沒有可變全域資料、硬體或 L03／L05 串接。
- 本次 Linux x86_64、GCC 14.2.0：sizeof Config=8、Input=8、State=56、Output=12 bytes；其他 ABI 可能不同。
- Init、Reset、Execute 均為 O(1)。Reset 不填滿或清零 buffer，只重設 index=filled=0、保存新參數與 initial_value／last_value；舊內容在新樣本逐格覆寫前不會被讀出。重設後首 N 次仍輸出新初值，不洩漏舊取樣期間資料。Reset 可以修復結構或 ring 數值損壞，但仍需由呼叫端提供有效儲存體。
- 不做樣本算術，只搬移有限 float。測試逐位元比對 ±FLT_MAX、±FLT_MIN、最小正／負次正規數、正負零；不得開啟 flush-to-zero／denormals-are-zero 等改變數值語意的模式。本次 Host 環境保留次正規值，未驗證 MCU FPU 設定。
- `__FAST_MATH__` 或 `__FINITE_MATH_ONLY__>0` 編譯期拒絕；其他不宣告巨集的非標準編譯器最佳化仍由整合端負責禁止。

## 測試摘要與逐項結果

2026-10-16：下列 21 個編號案例在 strict C99 -O0、-O2，以及 -O1 ASan+UBSan 全部 PASS。迴圈內的多個值／斷言不另算案例。測試來源：[test_fb_dead_time.c](../../../../../Tests/Unit/L04_FunctionBlock/test_fb_dead_time.c)。

| ID | 條件、預期與實際結果 | 結果 |
|---|---|---|
| UT-01 | 零初始化但未 Init 的 State 拒絕；Init 不寫入預填 NaN 的 ring，索引及 filled 歸零 | PASS |
| UT-02 | N=3，輸入 10/20/30 輸出 -7 且未 ready，第 4 次輸出 10 且 ready | PASS |
| UT-03 | 持續至第 100 次輸入，跨多圈精確輸出落後 3 筆資料，前後哨兵未變 | PASS |
| UT-04 | N=0、NULL buffer，第一筆即直通；負值也正確 | PASS |
| UT-05 | N=1 首筆初值，後續每筆精確輸出前一筆 | PASS |
| UT-06 | 穩態 sample_valid=false 分別搭配 NaN、±Inf，回 NO_SAMPLE、保留最後診斷值及所有位元組，恢復後續接原佇列 | PASS |
| UT-07 | 啟動期間缺樣不計數、不提早 ready；滿 N 次有效樣本後第 N+1 次才讀歷史 | PASS |
| UT-08 | 有效品質下 NaN、±Inf 各拒絕，State/ring 原子保持，再恢復有效輸入無偏移 | PASS |
| UT-09 | N 變動拒絕，缺樣也拒絕；恢復原設定後繼續原歷史 | PASS |
| UT-10 | Reset 更改 N、buffer、初值，舊／新 buffer 都未被 Reset 寫入；新期間不帶入舊歷史 | PASS |
| UT-11 | 相同 buffer Reset 不清除實體內容，但首 N 次都輸出新初值，之後只讀新樣本 | PASS |
| UT-12 | 容量不足、NULL 非零容量、capacity=SIZE_MAX、N=SIZE_MAX，Init/Reset 均拒絕且完整保持 State/ring | PASS |
| UT-13 | NaN／±Inf 初值、NULL config、NULL state；Init/Reset 回正確錯誤並保持既有資料 | PASS |
| UT-14 | Execute 四個指標各別 NULL，含 NULL output，無 State/ring 更新；可寫輸出提供無效診斷 | PASS |
| UT-15 | initialized=false、容量過小／極大、NULL buffer、索引越界、filled 越界、N 過大、非有限初值／末值、啟動 index/filled 不一致及不符初值，均拒絕 | PASS |
| UT-16 | 正要讀出的 ring 槽位各注入 NaN、±Inf，回 INVALID_STATE，拒絕發生在覆寫之前 | PASS |
| UT-17 | NO_SAMPLE 不讀損壞 ring；Reset 後重新填寫，無舊壞值外洩 | PASS |
| UT-18 | ±FLT_MAX、±FLT_MIN、正負零、最小正負次正規數，在 N=1 與 N=0 逐位元完全相同 | PASS |
| UT-19 | 不同 State/buffer 的兩實例互不影響，另一實例 Reset 和執行均不改本實例 | PASS |
| UT-20 | N=0 可提供未使用的普通 buffer 且完全不寫；缺樣凍結；非零 index 或 filled 拒絕 | PASS |
| UT-21 | capacity>N 時只寫前 N 格，額外容量及前後哨兵均未變 | PASS |

額外建置檢查：C++11 consumer 包含標頭並呼叫三個 API，連結 C99 object 通過；-ffast-math 與 -ffinite-math-only 各自觸發預期編譯拒絕。ASan+UBSan 使用 ASAN_OPTIONS=detect_leaks=0，無診斷；未執行 LeakSanitizer，不宣稱其通過。此獨立方塊測試不等同整個 repository CTest 或 MCU 整合測試。

## 重現命令

在 repository root 執行，需 GCC/G++ 與 libm：

```sh
mkdir -p /tmp/deadtime-tests
for opt in 0 2; do
  cc -std=c99 -Wall -Wextra -Werror -pedantic -O$opt \
    -I L04_FunctionBlock/Utility L04_FunctionBlock/Utility/FbDeadTime.c \
    Tests/Unit/L04_FunctionBlock/test_fb_dead_time.c -lm \
    -o /tmp/deadtime-tests/test-O$opt
  /tmp/deadtime-tests/test-O$opt
done
cc -std=c99 -Wall -Wextra -Werror -pedantic -O1 -g \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I L04_FunctionBlock/Utility L04_FunctionBlock/Utility/FbDeadTime.c \
  Tests/Unit/L04_FunctionBlock/test_fb_dead_time.c -lm -o /tmp/deadtime-tests/test-san
ASAN_OPTIONS=detect_leaks=0 /tmp/deadtime-tests/test-san

for flag in -ffast-math -ffinite-math-only; do
  if cc -std=c99 "$flag" -I L04_FunctionBlock/Utility \
    -c L04_FunctionBlock/Utility/FbDeadTime.c -o /tmp/deadtime-tests/reject.o; then
    echo 'ERROR: expected compilation rejection'; exit 1
  fi
done

cat > /tmp/deadtime-tests/link.cpp <<'CPP'
#include "FbDeadTime.h"
int main() {
    FbDeadTimeConfig c = {0}; FbDeadTimeState s = {};
    FbDeadTimeInput i = {2.0f,true}; FbDeadTimeOutput o = {};
    return FbDeadTime_Init(&c,&s,0,0,0) != FB_DEAD_TIME_OK ||
        FbDeadTime_Reset(&c,&s,0,0,1) != FB_DEAD_TIME_OK ||
        FbDeadTime_Execute(&c,&i,&s,&o) != FB_DEAD_TIME_OK || !o.ready;
}
CPP
cc -std=c99 -Wall -Wextra -Werror -pedantic -I L04_FunctionBlock/Utility \
  -c L04_FunctionBlock/Utility/FbDeadTime.c -o /tmp/deadtime-tests/fb.o
c++ -std=c++11 -Wall -Wextra -Werror -pedantic -I L04_FunctionBlock/Utility \
  /tmp/deadtime-tests/link.cpp /tmp/deadtime-tests/fb.o -lm -o /tmp/deadtime-tests/link
/tmp/deadtime-tests/link
```

## 限制與尚未驗證

不提供連續時間延遲、插值、動態記憶體管理、遺漏樣本補償或運作中 N 自動切換。不掃描整個 ring，不能保證偵測所有外部記憶體竄改。最大可實際延遲長度取決於目標 RAM；未配置超大型真實陣列、未量測 MCU 即時性，也未測試異常指標、重疊物件或資料競爭等違反 C 呼叫契約的未定義行為。Host 通過不代表產品安全認證或實機時序驗證。

## 獨立審查補充探測

另以非交付測試程式對 N=0..128 執行 129,000 次 FIFO／缺樣／邊界哨兵檢查，在 ASan+UBSan（detect_leaks=0）通過。這是額外審查，不併入上列 21 個正式案例。全體既有十個 FB 加本次兩個 FB 共十二套 strict C99 O0/O2 回歸通過；CMake/CTest 因環境缺少 cmake 未執行，不宣稱全平台或 MCU 驗證。

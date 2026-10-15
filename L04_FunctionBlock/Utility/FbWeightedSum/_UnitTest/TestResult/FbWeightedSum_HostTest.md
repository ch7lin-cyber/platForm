# FbWeightedSum Host Unit Test

## 摘要

- **測試結果：** PASS
- **測試案例：** 15 項
- **測試範圍：** 加權計算、有效輸入數量、未使用輸入槽、停用、非法設定／輸入、乘積及累加溢位、重設、空指標。
- **編譯標準：** C99；使用 `-Wall -Wextra -Werror -pedantic`
- **執行結果：** Host 測試程式結束碼為 0。
- **測試程式：** [test_fb_weighted_sum.c](../../../../../Tests/Unit/L04_FunctionBlock/test_fb_weighted_sum.c)

## 測試細則

| ID | 測試項目 | 設定／輸入 | 預期結果 | 驗證狀態 |
|---|---|---|---|---|
| UT-01 | 初始化 | 呼叫 `FbWeightedSum_Initialize` | input_count=4 | 通過 |
| UT-02 | 四路加權計算 | values=[1,2,3,4]；weights=[0.5,1,−1,2] | Run=true；sum=7.5；valid=true；status=OK | 通過 |
| UT-03 | 單路計算 | input_count=1；首路為 1×0.5 | Run=true；sum=0.5 | 通過 |
| UT-04 | 忽略未使用槽 | input_count=3；第 4 路 value=NaN | Run=true；sum=−0.5；不因未使用槽失敗 | 通過 |
| UT-05 | 輸入數量為零 | input_count=0 | Run=false；sum=0；valid=false；status=INVALID_CONFIGURATION | 通過 |
| UT-06 | 輸入數量超過上限 | input_count=5 | Run=false；status=INVALID_CONFIGURATION | 通過 |
| UT-07 | 有效槽 value 為 NaN | 第 1 路 value=NaN | Run=false；sum=0；status=INVALID_INPUT | 通過 |
| UT-08 | 有效槽 weight 為 Infinity | 第 1 路 weight=+Infinity | Run=false；status=INVALID_INPUT | 通過 |
| UT-09 | 停用 | Enable=false | Run=false；sum=0；valid=false；status=DISABLED | 通過 |
| UT-10 | 單項乘積溢位 | 第 1 路 1e30×1e30 | Run=false；sum=0；status=NONFINITE_RESULT | 通過 |
| UT-11 | 累加溢位 | 前兩路各為 2e38×1 | Run=false；sum=0；status=NONFINITE_RESULT | 通過 |
| UT-12 | 重設輸出 | 呼叫 `FbWeightedSum_Reset` | sum=0；valid=false；status=DISABLED | 通過 |
| UT-13 | Config 空指標 | config=NULL | Run=false；status=INVALID_ARGUMENT | 通過 |
| UT-14 | Input 空指標 | input=NULL | Run=false；status=INVALID_ARGUMENT | 通過 |
| UT-15 | Output 空指標 | output=NULL | Run=false；不解參考或寫入空指標 | 通過 |

### 共通錯誤輸出判定

對可寫入的 Output，錯誤案例皆檢查 sum 清為 0、valid=false，且 status 符合預期。Output 為 NULL 時檢查 Run 回傳 false。

### 尚未涵蓋的情況

- 有效輸入的零權重、全零輸入及正負項完全抵消。
- 輸入數量恰為 2 或 4 的獨立邊界案例。
- 有效槽 value 或 weight 為負 Infinity。
- 發生錯誤後再執行有效資料，確認輸出恢復有效。

## 重現方式

在 Platform repository 根目錄執行：

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic \
  -I L04_FunctionBlock/Utility \
  L04_FunctionBlock/Utility/FbWeightedSum.c \
  Tests/Unit/L04_FunctionBlock/test_fb_weighted_sum.c \
  -o test_fb_weighted_sum
./test_fb_weighted_sum
```

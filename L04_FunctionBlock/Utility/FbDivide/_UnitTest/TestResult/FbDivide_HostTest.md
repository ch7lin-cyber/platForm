# FbDivide Host Unit Test

## 摘要

- **測試結果：** PASS
- **測試範圍：** 18 個案例，涵蓋初始化、正負數相除、除零與 epsilon 判定、停用、非法浮點輸入／設定、結果溢位、重設及空指標。
- **編譯標準：** C99
- **警告選項：** `-Wall -Wextra -Werror -pedantic`
- **執行結果：** Host 測試程式結束碼為 0。
- **測試程式：** [test_fb_divide.c](../../../../../Tests/Unit/L04_FunctionBlock/test_fb_divide.c)

## 測試細則

| ID | 測試項目 | 設定／輸入 | 預期結果 | 驗證狀態 |
|---|---|---|---|---|
| UT-01 | 初始化 | 呼叫 `FbDivide_Initialize` | `denominator_epsilon = 0.0F` | 通過 |
| UT-02 | 正數相除 | Enable=true；12 ÷ 3 | Run=true；quotient=4；valid=true；status=OK | 通過 |
| UT-03 | 負數相除 | Enable=true；−12 ÷ −3 | Run=true；quotient=4；valid=true；status=OK | 通過 |
| UT-04 | 零除數 | epsilon=0；分母=0 | Run=false；quotient=0；valid=false；status=DIVIDE_BY_ZERO | 通過 |
| UT-05 | 正向 epsilon 邊界 | epsilon=0.01；分母=0.01 | 邊界值包含在拒絕範圍；status=DIVIDE_BY_ZERO | 通過 |
| UT-06 | 負向 epsilon 範圍內 | epsilon=0.01；分母=−0.005 | Run=false；status=DIVIDE_BY_ZERO | 通過 |
| UT-07 | epsilon 範圍外 | epsilon=0.01；−12 ÷ 0.02 | Run=true；quotient=−600；valid=true；status=OK | 通過 |
| UT-08 | 停用 | Enable=false | Run=false；quotient=0；valid=false；status=DISABLED | 通過 |
| UT-09 | NaN 分子 | numerator=NaN | Run=false；status=INVALID_INPUT | 通過 |
| UT-10 | 正無限大分母 | denominator=+Infinity | Run=false；status=INVALID_INPUT | 通過 |
| UT-11 | NaN epsilon | epsilon=NaN | Run=false；status=INVALID_CONFIGURATION | 通過 |
| UT-12 | 負 epsilon | epsilon=−1 | Run=false；status=INVALID_CONFIGURATION | 通過 |
| UT-13 | 正無限大 epsilon | epsilon=+Infinity | Run=false；status=INVALID_CONFIGURATION | 通過 |
| UT-14 | 商值溢位 | 1e30 ÷ 1e−30 | Run=false；quotient=0；valid=false；status=NONFINITE_RESULT | 通過 |
| UT-15 | 重設輸出 | 呼叫 `FbDivide_Reset` | quotient=0；valid=false；status=DISABLED | 通過 |
| UT-16 | Config 空指標 | config=NULL，其餘有效 | Run=false；status=INVALID_ARGUMENT | 通過 |
| UT-17 | Input 空指標 | input=NULL，其餘有效 | Run=false；status=INVALID_ARGUMENT | 通過 |
| UT-18 | Output 空指標 | output=NULL | Run=false；不解參考或寫入空指標 | 通過 |

### 共通錯誤輸出判定

對可寫入的 Output，錯誤案例皆檢查：

- `quotient == 0.0F`
- `valid == false`
- `status` 符合該案例預期

Output 為 NULL 時，只能檢查 Run 回傳 false。

### 尚未涵蓋的情況

以下情況不在本次 18 個測試案例內：

- 分子為正／負 Infinity，以及分母為負 Infinity。
- 分母為 −0.0F、負 epsilon 邊界等值。
- `FbDivide_Initialize(NULL)` 與 `FbDivide_Reset(NULL)`。
- 發生錯誤後再次輸入有效值，確認輸出可恢復為有效狀態。

## 重現方式

在 Platform repository 根目錄執行：

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic \
  -I L04_FunctionBlock/Utility \
  L04_FunctionBlock/Utility/FbDivide.c \
  Tests/Unit/L04_FunctionBlock/test_fb_divide.c \
  -o test_fb_divide
./test_fb_divide
```

# FbPidGroupSelector Host 驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_pid_group_selector.c 已以 C99、Wall/Wextra/Werror、ASan／UBSan 編譯執行通過。LeakSanitizer受環境/proc限制，以detect_leaks=0執行；本FB無heap配置。CMake／CTest入口已加入，但環境未安裝工具，未宣稱該路徑或MCU驗證通過。

## 測試細則

| 測項 | 預期與結果 |
|---|---|
| 合法預設 | DefaultConfig可完整驗證，通過 |
| 固定四組 | 四組各給不同的十個參數，逐欄核對輸出副本；SV無效不影響固定選組，通過 |
| Auto邊界 | 門檻-1000／0／1000，測INT_MIN、各等號、相鄰及INT_MAX，通過 |
| 相同門檻 | 三個0，SV=-1選0、SV=0選3，通過 |
| Auto無效SV | 清零並INVALID_INPUT，通過 |
| 外部鎖組 | 無效SV仍可鎖第3組；locked_group=4拒絕，通過 |
| 全組驗證 | 未選組cool_pb=0亦拒絕；反序門檻、selection=5拒絕，通過 |
| 範圍驗證 | 時間上限+1、係數201、IOF1001、PB上限+1拒絕，通過 |
| 邊界合法 | PB=1／3000000、時間0／30000000、IOF1000、係數0／200接受，通過 |
| 值副本／無狀態 | Config修改不回頭改輸出，下一次呼叫取得新值，通過 |
| 停用 | 忽略非法配置並清零DISABLED，通過 |
| NULL | 必要參數判錯，DefaultConfig(NULL)安全返回，通過 |

尚未驗證：CKLIB真正接受的範圍與換算、整組套用失敗回復、運轉中切組的MV連續性、AT讀回／回寫、NVM與MCU。

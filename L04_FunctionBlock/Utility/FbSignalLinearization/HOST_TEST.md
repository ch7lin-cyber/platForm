# 輸入／輸出折線 Host 驗證

## 摘要

測試來源：Tests/Unit/L04_FunctionBlock/test_fb_signal_linearization.c。C99、Wall/Wextra/Werror、ASan／UBSan 通過。容器 LeakSanitizer 無法讀取 /proc task，因此以 ASAN_OPTIONS=detect_leaks=0 執行；本模組不配置 heap。原 FbLinearization 59 案回歸通過。本報告不代表 MCU 或設備相容性測試。

## 細則

CMake library 與 CTest target 已加入；本環境未安裝 cmake／ctest，故以 cc 直接編譯執行上述來源，未宣稱 CMake 建置通過。

| 測項 | 預期及結果 |
|---|---|
| Value | x=-5，表 (-10,10)/(0,20)/(10,-10)，輸出 15，通過 |
| Deviation + Offset | -5+15+2=12，通過 |
| Bypass | 即使點表 NULL，原值 -5 有效通過，通過 |
| 左端步階與節點 | -11/-10/-5/0/5/10/11 對應 10/10/10/20/20/-10/-10，通過 |
| X 域 ERROR | x=-11 回 OUT_OF_RANGE，清零失效，通過 |
| Offset 後限制 | 端點10+5=15，夾至12並標 limited，保留 BELOW，通過 |
| 輸出限制 ERROR／反序限制 | 分別 OUT_OF_RANGE／INVALID_CONFIGURATION，通過 |
| 重複 X | 拒絕，通過 |
| 無效品質／NaN | INVALID_INPUT，通過 |
| OFF／enable=false | 忽略無效數值／模式，回零且 disabled，通過 |
| NULL／不合法模式 | INVALID_ARGUMENT／INVALID_CONFIGURATION，通過 |
| 加總溢位 | FLT_MAX+FLT_MAX 拒絕；明確限制至100則成功，通過 |

尚待產品驗證：單位路由、固定容量配置、Register／NVM 套用、MCU 精度與時序、既有 Safety／實體輸出串接。

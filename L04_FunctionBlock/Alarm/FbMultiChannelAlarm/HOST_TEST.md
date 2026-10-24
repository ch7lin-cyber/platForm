# FbMultiChannelAlarm Host 驗證

## 摘要

來源：Tests/Unit/L04_FunctionBlock/test_fb_multi_channel_alarm.c。C99、Wall/Wextra/Werror、ASan／UBSan 直接編譯執行通過。環境 LeakSanitizer 受 /proc 存取限制，detect_leaks=0；FB 無 heap 配置。CMake／CTest target 已加入，但此環境無該工具，未宣稱 CMake 或 MCU 驗證通過。

## 測試細則

| 測項 | 斷言與結果 |
|---|---|
| 選取／極值 | 兩選取PV=100000／102000，極值正確、差2000；未選取的失效 INT_MIN 不影響，通過 |
| 延遲 | 首筆0，50ms後未成立，再50ms成立並單次 request，後續不重送，通過 |
| 超大 elapsed | UINT_MAX 飽和至10000，不溢位，通過 |
| 失效恢復去重 | 已觸發後失效／恢復，重新延遲但不再 request，通過 |
| 未完成延遲取消 | 尚在計時時失效，恢復首筆重新從0，通過 |
| 門檻等號／自動武裝 | spread==limit正常；再次超差可新觸發，通過 |
| Hold | 條件正常及資料失效不清除 latch，資料失效時 valid=false，通過 |
| Reset | 無效／條件成立時拒絕；持續高不重試；拉低再拉高且正常才接受，通過 |
| 設定 revision | 更新建立新週期，允許一次新動作，通過 |
| 實例獨立 | 第二實例不沿用第一實例的時間，通過 |
| 停用 | 即使設定錯誤仍 DISABLED 且清除全部狀態，通過 |
| 配置／資料錯誤 | 超時限、少於兩選取、count不符、NULL samples，通過 |
| 最大整數差 | INT_MIN與INT_MAX差4294967295；等號正常、低一點觸發，通過 |
| 頂層 NULL | 各必要參數缺漏均 INVALID_ARGUMENT，通過 |

尚待驗證：L05 跨模組 provider／時間品質、SYSTEM 動作佇列與結果回報、Register／NVM、MCU。

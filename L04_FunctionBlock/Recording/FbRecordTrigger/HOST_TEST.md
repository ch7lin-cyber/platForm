# FbRecordTrigger Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_record_trigger.c已以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受/proc限制，以detect_leaks=0執行，模組無heap。CMake／CTest入口已加入，環境無工具，未宣稱CMake或MCU驗證通過。

## 細則

| 測項 | 結果 |
|---|---|
| 初始週期 | 首次9999ms前置時間忽略，從0開始 |
| 週期相位／漏期 | 500+500到期；3500ms跨3次只請求一筆、missed2並留500相位 |
| 事件延遲 | edge首筆0，兩個100ms後200ms到期 |
| 冷卻 | 300ms內邊緣忽略，到期後新edge可接受 |
| 失效恢復 | 取消pending，恢復高位準不創造新edge |
| 同次來源合併 | 週期與事件各成立，單一request且reasons兩bit |
| 單次武裝 | 一次事件後不再發事件，週期仍可發；Reset重新武裝但先種入位準 |
| 事件fault隔離 | event_fault=true時週期請求仍成立 |
| Falling／Both | 初始高不發；下降／雙邊緣按設定觸發 |
| uint32時間邊界 | 事件delay=UINT32_MAX正常到期；period1的大dt回報UINT32_MAX−1漏期 |
| 實例獨立 | 第二實例從0相位，不沿用第一實例 |
| 配置／停止／disable／NULL | period0拒絕，停止或disable清除狀態，必要NULL安全判錯 |

尚待驗證：L05事件來源、實際擷取／保存回報、佇列滿載及重試、Register／NVM與MCU。

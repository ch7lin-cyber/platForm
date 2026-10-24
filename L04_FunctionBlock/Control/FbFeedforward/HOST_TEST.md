# FbFeedforward Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_feedforward.c，以C99、Wall/Wextra/Werror及ASan／UBSan編譯執行通過，連結libm。LeakSanitizer受環境/proc限制，detect_leaks=0；本FB無heap配置。CMake／CTest入口已加入，但環境無工具，未宣稱CMake或MCU驗證通過。

## 細則

| 測項 | 預期與結果 |
|---|---|
| Delay與首筆時間 | 首個edge忽略999ms前置dt，delay從0開始 |
| 兩步階與負衰減 | 第一段100、第二段-100、Decay中點-50、結束0 |
| 完成／重新觸發 | 完成脈衝一次；持續高不再啟動；低→高可重啟 |
| 事件低／Reset | Tx0立即取消；Reset高後保持trigger高不自動再啟動 |
| Force含delay | force50ms<delay100ms時不產生非零值 |
| 強制執行與截止 | trigger提早低仍執行，350ms時硬歸零；high也不延長 |
| 正常波形先結束 | total700ms<force1000ms，正常completed，沒有額外保持 |
| 執行中edge | 不重啟elapsed，後續既定時間正確 |
| 正規化指數 | T=tau=1000，中點0→1000約622；第二段1000→-1000中點約-244；端點正確 |
| 零時間 | 跳過第一段非零target，第二指數段從0；全部零無補償脈波 |
| 大dt | UINT_MAX可直接正常結束，不溢位／不迴圈 |
| 配置改變／來源失效 | 取消並歸零，恢復後需新edge |
| 實例獨立 | 第二實例從0時間啟動 |
| 配置／生命週期 | 指數tau0、target1501、delay上限+1拒絕；合法預設、disable與NULL安全處理 |

以上斷言均通過。SYSTEM事件、L05 FF＋PID、熱冷分配／anti-windup、Register／NVM及MCU尚待接入。

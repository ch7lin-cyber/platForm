# FbProcessResponse Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_process_response.c以C99、Wall/Wextra/Werror及ASan／UBSan編譯執行通過。LeakSanitizer受/proc限制，detect_leaks=0；FB無heap。CMake／CTest入口已加入，環境無工具，未宣稱CMake或MCU驗證通過。

## 細則

| 測項 | 斷言與結果 |
|---|---|
| 升向時間 | span1000，PV100／632／900於100／200／300ms到達；onset100ms |
| 穩定及增益 | 1000／950／1050三筆含band等號，200ms後確認，mean1000、gain10、overshoot50 |
| 完成凍結 | 完成後時間／結果不再變動 |
| 降向／負MV步階 | final=-1000、MV step=-100，方向時間與正gain10正確 |
| 重複序號 | 不增加穩定樣本數，不提前完成 |
| 出帶恢復 | PV=-1100出帶後，重新滿足三筆與dwell才完成 |
| 失效／實驗改變 | valid=false鎖FAULT；MV偏差超限拒絕，在容許等號可觀測 |
| Config改變 | 觀測中改onset_change鎖FAULT，不能混用條件 |
| 逾時 | UINT_MAX dt安全判TIMEOUT；exact timeout達成穩定可完成 |
| 取消 | STOP保留部分診斷但result_valid=false |
| 整數極值／序號回繞 | INT_MIN→INT_MAX的幅度與增益有限，UINT_MAX→0視為新樣本 |
| 獨立實例 | 第二實例time0、sample1，不沿用第一實例歷史 |
| 基準／配置／disable／NULL | 無效基準及非法min step拒絕；disable清零，必要參數安全判錯 |

尚待產品驗證：穩定基準來源、實際MV回報、時序品質、負載變動、L05實驗協調、結果保存／UI與MCU。

# FbBatchMetrics Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_batch_metrics.c，以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受環境/proc限制，以detect_leaks=0執行；模組無heap配置。CMake／CTest入口已加入但環境無工具，未宣稱CMake或MCU驗證通過。

## 細則

| 測項 | 結果 |
|---|---|
| 新樣本統計 | 900／1100兩筆mean1000、min900、max1100、variance10000 |
| 重複序號 | 不增加筆數，仍依有效保持樣本累計時間／偏差積分 |
| 起始時間 | START忽略先前999ms，第一筆time0 |
| 偏差積分 | 目標1000、前值900，兩個500ms區間signed=-100、absolute=100 |
| 到溫連續 | band100含等號，1000ms後reached，arrival1000 |
| 鎖定SV | 運行時輸入SV改變及sv_valid=false不改target |
| 資料失效／恢復 | 保留歷史統計，invalid_calls增加；失效與恢復首區間不積分 |
| 批次結束／重新命令 | STOP保留結果不再計時；RESET清除；無效SV拒絕START、held START不重啟 |
| 到溫中斷 | invalid取消連續時間，恢復需重新滿足dwell |
| 冷卻過衝／整數邊界 | INT_MAX目標與INT_MIN PV過衝4294967295，誤差不溢位 |
| 序號回繞與統計 | UINT_MAX→0計新樣本，INT_MIN／INT_MAX平均-0.5，variance有限 |
| 實例獨立／配置改變 | 第二實例從time0；改Config進FAULT且拒絕直接START |
| 計數溢位 | elapsed=UINT64_MAX再加1進FAULT，不回繞 |
| 停用／NULL | 清零且DISABLED，必要參數缺漏安全判錯 |

尚待驗證：L05批次事件／樣本品質、產品保存與UI、MCU資源及實際量測資料。

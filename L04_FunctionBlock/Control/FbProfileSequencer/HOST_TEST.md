# FbProfileSequencer Host 驗證

## 摘要

來源Tests/Unit/L04_FunctionBlock/test_fb_profile_sequencer.c，以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受環境/proc限制，detect_leaks=0；模組無heap配置。CMake／CTest入口已加入，環境無cmake／ctest，未宣稱該路徑或MCU驗證通過。

## 細則

| 測項 | 結果 |
|---|---|
| Ramp／Soak／下降段 | 0→100000／1000ms，500ms得50000；等值持溫及100000→0下降均正確 |
| 跨段時間 | 剩餘250ms移入Soak，再跨至下降段；SV／步號／時間正確 |
| HOLD／RESUME | 保留段時間與SV，hold期間10000ms不追補；等待階段亦可暫停 |
| 完成與命令edge | END保持最後SV；START持續高不在完成後重啟 |
| 起始步／ADVANCE／STOP | 指定step1；跳步立即到目標；STOP請求只一個呼叫 |
| 重複／連接 | 額外重複一次後連到第二樣式，再STOP；剩餘時間正確 |
| 狀態獨立 | 第二實例從initial_sv與0時間開始 |
| 等待帶等號 | abs差=band可通過；既有等待時間不回算下一段 |
| 等待逾時／PV失效 | 一次停止／逾時請求；FAULT保持且拒絕START；RESET可清除 |
| timeout=0 | 無限等待且累計飽和UINT32_MAX |
| 起始斜率 | 60000milli-C/min，100000差需100000ms，中點正確 |
| 最大整數範圍 | INT_MIN→INT_MAX搭配UINT32_MAX時間不溢位 |
| 起始時間溢位 | 斜率1時無法以uint32時間表示，安全拒絕 |
| 零時間循環 | 自連零時間樣式觸發轉移上限，不阻塞 |
| 配置變更／無效起點 | revision改變FAULT；非法start_step拒絕 |
| disable／NULL | disable清除至IDLE，NULL必要參數安全判錯 |

尚待產品測試：配方設定交易、NVM斷電恢復、L05動態SV與PID／Cascade串接、SYSTEM動作處理及MCU。

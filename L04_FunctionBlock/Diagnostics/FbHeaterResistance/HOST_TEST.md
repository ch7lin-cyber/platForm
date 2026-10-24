# FbHeaterResistance Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_heater_resistance.c已以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受/proc限制，以detect_leaks=0執行，模組無heap。CMake／CTest入口已加入，但環境無工具，未宣稱該路徑或MCU驗證通過。

## 細則

| 測項 | 斷言結果 |
|---|---|
| 基本單位 | 230000mV／10000mA=23000mΩ、2300000mVA |
| 基準偏移 | 20Ω基準得+150‰，25Ω基準得-80‰ |
| 界限 | 等界不報；低於／高於界限正確旗標且估值仍valid |
| 資料時效 | age100ms與skew10ms等號接受，超過拒絕 |
| 關閉age限制 | age=UINT_MAX仍可在允許skew內計算，不發生相減溢位 |
| 來源品質 | 任一valid=false拒絕，輸出清零 |
| 低量測／停電 | 0V先LOW_VOLTAGE，有V但0A為LOW_CURRENT；未供電INACTIVE |
| 捨入 | 1mV／16mA=62.5mΩ向上63，0.5mVA向上1 |
| 整數上限 | 最大uint32電壓／1mA達MAX阻值，基準差倍率不溢位；最大V×I產品正確 |
| 無效設定／disable／NULL | 基準0或最低電流0拒絕；disable清零，必要參數安全判錯 |

尚待產品驗證：Vrms／Irms量測與時間窗、PWM／SCR供電狀態、校正及溫度係數、L05／SYSTEM診斷政策、MCU。

# FbPowerVoltageCompensation Host驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_power_voltage_compensation.c以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受/proc限制，detect_leaks=0，模組無heap。CMake／CTest入口已加入，環境無工具，未宣稱CMake／MCU驗證通過。

## 細則

| 測項 | 結果 |
|---|---|
| 預設旁路 | 無效電壓仍可旁路有效MV400 |
| 額定電壓 | 230V／230V倍率1、MV不變 |
| 平方增減 | 230／115倍率4；230／460倍率0.25 |
| 倍率界限 | 更低／更高有效電壓受0.25～4倍限制 |
| MV上限 | 需求1600限至1000；自訂123與0上限正確 |
| 零MV | 有效電壓下保持0，不產生最低功率 |
| 品質／時效 | 0V為TOO_LOW，age100等號接受、101拒絕；0可關閉age限制 |
| 無效MV | 品質失效、負值、1001拒絕 |
| 全數值範圍 | UINT_MAX額定／1mV的raw gain有限，先限倍率；反向極值亦正常 |
| 捨入 | 強制1.5倍，MV1得到2 |
| 配置／disable／NULL | 倍率0／反序拒絕；disable清零，必要參數安全判錯 |

尚待產品驗證：供電電壓量測點、PWM／SCR功率比例模型、輸出鏈順序與anti-windup、Register／NVM與MCU。

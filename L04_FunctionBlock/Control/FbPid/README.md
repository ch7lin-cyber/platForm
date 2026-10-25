![FbPid 方塊圖：輸入、輸出、內部功能與參數設定](FbPid_BlockDiagram.svg)

# FbPid 溫度單位相容邊界

## 摘要

2026-10-25：公開FbPidInput_t與L05 provider的PV／SV統一採int32、0.001°C。目前CKLIB仍使用0.1°C，暫時在FbPid.c的ToCklibTemperature單一邊界轉換，L05不得先除100。MV保持0.1%，時間與SetParameter設定值不因這次PV／SV引腳變更而自動改刻度。

**下一版待Colin將CKLIB的溫度介面統一為0.001°C；實際版本／能力確認後才移除相容轉換。** 此為待辦，不是目前後端已支援或可達0.001°C控制解析度的宣告。

## 方塊圖與引腳

2026-10-26：SVG 範例依 `FbPid.h` 的正式介面繪製。外框代表單一 `FbPid_t` 實例，左側為每次 Execute 的輸入，右側為輸出；內部虛線框為功能分組。底部設定框對應 Init／SetParameter API，並非每次執行的輸入引腳。內部線條表示功能關係，詳細執行順序仍以程式為準。

| 方向 | 欄位 | 型別／單位 | 說明 |
|---|---|---|---|
| 輸入 | pv、sv | int32；0.001°C | 量測值、設定值；L04 暫轉 CKLIB 0.1°C |
| 輸入 | enable | bool | 禁能時 MV 歸零，輸出 valid=false |
| 輸入 | reset | bool | 上升沿重置命令 |
| 輸入 | valid | bool | 輸入資料有效性 |
| 輸入 | elapsed_ms | uint32；ms | 本次執行的時間步長 |
| 輸出 | mv[2] | int32；0.1% | MV1／MV2，0～1000 |
| 輸出 | mv_count | uint8 | 後端回傳的有效 MV 數量 |
| 輸出 | valid | bool | 運算輸出有效性 |
| 輸出 | auto_tune_completed | bool | 自整定完成並移交控制器 |
| 輸出 | status | FbPidStatus_t | FB 執行狀態 |
| 輸出 | backend_error | int32 | CKLIB 錯誤碼 |

設定包含四種模式、控制方向、AT 目標 PID 模式與週期／容許誤差；個別 PID／ON_OFF／AT 參數透過 `FbPid_SetParameter()` 設定，其適用模式與刻度須依 CKLIB 規格。SVG 可隨程式版本一起維護；修改公開介面時須同步更新圖與本表。

## 目前轉換規則

PV／SV以最接近0.1°C取整，正好半格時離零；int64中間值避免INT32_MIN取負或加捨入量溢位。

| 公開原值 | 溫度 | CKLIB原值 |
|---|---|---|
| 25123 | 25.123°C | 251（25.1°C） |
| 25150 | 25.150°C | 252（25.2°C） |
| -25150 | -25.150°C | -252（-25.2°C） |
| 49／50 | 0.049／0.050°C | 0／1 |

ON_OFF／PID／PID_FB經控制器PV／SV設定使用同一轉換；AUTO_TUNE的SetSV／CollectAndSignal亦同。AT結束後交回控制器時，仍以公開原輸入經同一邊界轉一次，不重複除100。

## Colin下一版遷移清單

- [ ] Colin提供實際CKLIB新版與檔頭／參數單位規格，確認PV／SV為0.001°C。
- [ ] 核對PB／冷卻PB、Deadband、ON_OFF感度及AT相關溫度設定，不只改PV／SV。
- [ ] 移除ToCklibTemperature縮放，或依明確後端版本選擇相容模式；禁止只換.a而保留舊縮放。
- [ ] 同步修改PID、ON_OFF、PID_FB與AT mock期望值，驗證AT移交不重複換算。
- [ ] 更新產品參數轉換／文件並以實體庫及MCU確認。

舊呼叫端若曾以250表示25°C，現在必須傳25000；不得沿用舊值。正式PID provider尚未接入，Host測試已改用milli°C，並新增正負半格、INT32極值與AT移交驗證，ASan／UBSan通過。實體.a仍為空檔佔位，數值控制效果未驗證。

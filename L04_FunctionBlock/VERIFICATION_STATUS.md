# Function Block 完成狀態與驗證用語

## 摘要

2026-10-27：統一文件用語。**24 個 L04 運算模組已完成實作並有 Host 單元測試通過紀錄；FbPid 已完成 L04 封裝並有 Host mock 單元測試通過紀錄，實體 CKLIB 尚未驗證。** 產品串接與 MCU 實測另列，不能由前述結果推定完成。FbC7gLogic 尚未實作。

本次是文件狀態整理，沿用各報告既有測試紀錄，沒有重新執行全部測試。測試日期、編譯條件、案例數與 sanitizer 結果以各報告為準；不能推定每個模組都執行過相同工具或相同組態。

## 各項名稱的意義

| 項目 | 定義 | 完成判定與限制 |
|---|---|---|
| L04 實作 | 公開介面、算法／封裝、狀態與錯誤處理已寫入程式 | 依本專案公開契約判定；不等於完全相容 DTDM04，也不代表已執行測試 |
| 單元測試 | 針對單一 FB 定義輸入、預期結果及判定條件 | 「案例已建立」與「執行通過」需區分；只寫好測試程式不能標為通過 |
| Host 單元測試通過 | FB 與測試程式在電腦上編譯、執行，符合既定預期 | 「Host」是執行環境，「單元測試」是測試範圍，兩者不是兩套獨立完成項目 |
| Host mock 單元測試通過 | 在電腦上用模擬後端檢查封裝呼叫、參數傳遞與錯誤處理 | FbPid 使用此標記；不能證明實體 CKLIB 的 PID 算法或閉迴路控制效果 |
| Host 整合測試通過 | 在電腦上檢查多個模組的資料傳遞、排程與串接 | 只證明測試涵蓋的路徑；不代表全部通道、Register、NVM 或實體輸出完成 |
| MCU 實測通過 | 在指定目標板、韌體組態及硬體條件下執行並留下結果 | 需明列測項、平台與結果；Host 結果不直接作為 MCU 實測結果 |

ASan／UBSan 是測試執行時的補充檢查工具，不是另一種功能完成度；工具未報錯不代表已涵蓋所有記憶體、時序或數值情境。CMake／CTest 入口存在也不等於已由該入口執行通過。

## 統一狀態寫法

一般算法使用：**L04 實作完成；Host 單元測試通過；產品串接與 MCU 實測另列狀態。**

PID 使用：**L04 封裝完成；Host mock 單元測試通過；實體 CKLIB 與 MCU 驗證未完成。**

單獨寫「已完成」時，必須指出完成範圍。產品串接可標為「部分完成」並寫出已接路徑；不能將已有部分串接寫成全部未做，也不能以局部測試標為全產品完成。下表的 MCU 狀態指各 FB 的目標板驗證，不沿用其他 SYSTEM／Safety 測試結果。

## 各 FB 狀態

| FB | 實作 | Host 單元測試 | 產品串接 | MCU 實測 |
|---|---|---|---|---|
| [FbAlarm](Alarm/FbAlarm/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Alarm/FbAlarm/_UnitTest/TestResult/FbAlarm_HostTest.md) | 部分完成：L05 Alarm／Logic adapter 與 Host 整合測試已有紀錄；Register／NVM／實體輸出未完成 | 尚未實測 |
| [FbMultiChannelAlarm](Alarm/FbMultiChannelAlarm/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Alarm/FbMultiChannelAlarm/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbCascadeCoordinator](Control/FbCascadeCoordinator/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbCascadeCoordinator/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbFeedforward](Control/FbFeedforward/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbFeedforward/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbOutputLimiter](Control/FbOutputLimiter/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbOutputLimiter/_UnitTest/TestResult/FbOutputLimiter_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbPid](Control/FbPid/README.md) | L04 封裝完成 | Host mock 單元測試通過；[產品 mock 紀錄](https://github.com/ch7lin-cyber/myNXPs16_Prj/blob/branch2_ver1/_Product/Docs/L05_PID_CONTROLLER_ADAPTER.md) | 部分完成：L05 adapter 與 mock 串接已有測試紀錄；正式 PID provider／callback、實體庫及輸出鏈未完成 | 尚未實測 |
| [FbPidGroupSelector](Control/FbPidGroupSelector/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbPidGroupSelector/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbPowerVoltageCompensation](Control/FbPowerVoltageCompensation/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbPowerVoltageCompensation/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbProfileSequencer](Control/FbProfileSequencer/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbProfileSequencer/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbSetpointRamp](Control/FbSetpointRamp/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Control/FbSetpointRamp/_UnitTest/TestResult/FbSetpointRamp_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbBatchMetrics](Diagnostics/FbBatchMetrics/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Diagnostics/FbBatchMetrics/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbHeaterResistance](Diagnostics/FbHeaterResistance/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Diagnostics/FbHeaterResistance/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbProcessResponse](Diagnostics/FbProcessResponse/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Diagnostics/FbProcessResponse/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbRecordTrigger](Recording/FbRecordTrigger/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Recording/FbRecordTrigger/HOST_TEST.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbC7gLogic](Utility/FbC7gLogic/README.md) | 尚未實作 | 尚未建立／執行；無 | 尚未串接 | 尚未實測 |
| [FbDeadTime](Utility/FbDeadTime/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbDeadTime/_UnitTest/TestResult/FbDeadTime_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbDivide](Utility/FbDivide/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbDivide/_UnitTest/TestResult/FbDivide_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbInBandTimer](Utility/FbInBandTimer/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbInBandTimer/_UnitTest/TestResult/FbInBandTimer_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbInputSwitchOver](Utility/FbInputSwitchOver/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbInputSwitchOver/HOST_TEST.md) | 部分完成：CH0 PV 處理 adapter 已提供可選切換及 Host 整合測試；正式多通道／Register 整合未完成 | 尚未實測 |
| [FbLeadLag](Utility/FbLeadLag/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbLeadLag/_UnitTest/TestResult/FbLeadLag_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbLinearization](Utility/FbLinearization/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbLinearization/_UnitTest/TestResult/FbLinearization_HostTest.md) | 部分完成：由輸入／輸出折線包裝重用，CH0 PV 路徑已有 Host 整合測試紀錄；完整產品控制鏈未完成 | 尚未實測 |
| [FbLogic](Utility/FbLogic/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbLogic/_UnitTest/TestResult/FbLogic_HostTest.md) | 部分完成：L05 多組邏輯與排程已有 Host 整合測試紀錄；正式 PID／Register／實體輸出未完成 | 尚未實測 |
| [FbMovingAverage](Utility/FbMovingAverage/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbMovingAverage/_UnitTest/TestResult/FbMovingAverage_HostTest.md) | 部分完成：CH0 Snapshot→L05→L04 唯讀鏈已有 Host 整合測試紀錄；未接實體輸出 | 尚未實測 |
| [FbSelector](Utility/FbSelector/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbSelector/_UnitTest/TestResult/FbSelector_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |
| [FbSignalLinearization](Utility/FbSignalLinearization/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbSignalLinearization/HOST_TEST.md) | 部分完成：CH0 PV 處理與 Alarm 可選處理後 PV 路徑已有 Host 整合測試紀錄；Register／完整控制鏈未完成 | 尚未實測 |
| [FbWeightedSum](Utility/FbWeightedSum/README.md) | L04 實作完成 | Host 單元測試通過；[報告](Utility/FbWeightedSum/_UnitTest/TestResult/FbWeightedSum_HostTest.md) | 尚未完成；詳細待辦見本模組文件與產品整合文件 | 尚未實測 |

## 證據與更新方式

- 各 README 的摘要顯示同一組狀態，測試報告保留原始執行條件、日期及案例細則。
- 產品串接由主專案 L05 文件追蹤；SYSTEM／Safety 仍保留最終輸出權責。
- 更新狀態須有相應程式、測試或上板紀錄，不以新增文件或畫出 FBD 作為功能通過的證據。
- 本頁與各 README 是目前彙整；舊測試報告的 PASS 僅對應該報告的日期、版本與範圍。

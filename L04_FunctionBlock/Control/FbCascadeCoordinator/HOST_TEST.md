# FbCascadeCoordinator Host 驗證

## 摘要

Tests/Unit/L04_FunctionBlock/test_fb_cascade_coordinator.c 已以C99、Wall/Wextra/Werror及ASan／UBSan直接編譯執行通過。LeakSanitizer受環境/proc限制，以detect_leaks=0執行；FB無heap配置。CMake／CTest入口已加入，未安裝該工具的環境未驗證CMake路徑或MCU。

## 細則

| 測項 | 結果 |
|---|---|
| 預設設定 | ValidateConfig接受 |
| 偏差模式 | 主控220°C、偏差-20～+40°C，MV0／500／1000映射200／230／260°C |
| MV超界 | -1／1001夾制並標記，不溢位 |
| 反向映射 | MV0對應高SV，MV1000對應低SV |
| 絕對模式 | 不需有效master_sv；中點加Offset正確 |
| Offset限幅 | 正負Offset超界被夾住並標target_clamped |
| 濾波 | 首筆種入0；dt=tau時100000目標分別得50000、75000；dt0保持 |
| 實例獨立 | 另一實例首筆直接種入100000，不繼承歷史 |
| 無效恢復／設定變更 | 清除舊濾波，恢復或變更後重新種入 |
| 移動界限 | 偏差基準改變且dt0，歷史仍受新界限限制 |
| 相對加總溢位／品質 | 界限超出int32或master_sv無效拒絕 |
| 等界／全整數範圍 | SV上下界相同可用；完整INT_MIN～INT_MAX映射端點正確 |
| 設定錯誤 | MV上下界相同、非法模式拒絕 |
| disable／reset／NULL | 正確清零／狀態與必要參數檢查 |

以上斷言均通過。正式雙PID、AT／校準、anti-windup／副控保護、Register／NVM與MCU待接入。

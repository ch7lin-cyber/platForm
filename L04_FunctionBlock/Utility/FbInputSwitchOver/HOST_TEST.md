# FbInputSwitchOver Host 驗證

## 摘要

來源：Tests/Unit/L04_FunctionBlock/test_fb_input_switch_over.c。C99、Wall/Wextra/Werror 直接編譯並以 ASan／UBSan 執行通過。環境不支援 LeakSanitizer 的 /proc 存取，故 detect_leaks=0；模組沒有 heap 配置。已加入 CMake／CTest target，但環境無 cmake／ctest，未宣稱該路徑或 MCU 驗證通過。

## 測試細則

| 範圍 | 案例與預期 |
|---|---|
| 低門檻等號／優先序 | PV1=200000、PV2=900000，選 SOURCE1 |
| 高門檻等號 | PV1=200001、PV2=400000，選 BOTH，平均 300000 |
| 高門檻超出 | PV2=400001，選 SOURCE2 |
| 全故障遮罩 | USE_OTHER／KEEP 各驗證 00／01／10／11，包含 BOTH 選擇期間失效與恢復 |
| 維持失效來源 | SOURCE1 故障不使用上次 PV，回零／無效且 selected=SOURCE1 |
| 首次故障／實例獨立 | 另一實例無歷史，KEEP 從 SOURCE1 開始；健康來源1可在來源2故障時維持有效 |
| 設定變更 | 原選 SOURCE2，修改門檻後重設至初始選擇，不沿用舊歷史 |
| Sensor 範圍 | 端點有效；低於下限來源標 NG，切到另一來源並標 degraded |
| 整數邊界 | INT_MAX+INT_MAX 平均不溢位；-3與-2朝零得-2 |
| 相同切換門檻 | 接受且依正常次序確定選擇 |
| 不合法設定 | 反序門檻與不合法 fail_action 拒絕，清除歷史 |
| ENABLE／Reset | 停用優先、Reset 當次零無效、無歷史殘留 |
| NULL 參數 | 四種必要指標缺漏皆 INVALID_ARGUMENT |

以上斷言均通過。L05 時效／來源路由、Register 套用、NVM、實體 MCU 與故障注入尚未接入。

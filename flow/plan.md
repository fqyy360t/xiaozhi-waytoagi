# Plan — EasyInput V2 适配 xiaozhi-esp32

## 板型信息
- type/name: `easyinput-v2`，target: `esp32s3`，flat 布局 `main/boards/easyinput-v2/`（单板，不声明 manufacturer）。
- Flash 16MB / PSRAM 8MB Octal：仓库根 `sdkconfig.defaults` 已默认，config.json 不重复写。

## 已完成（v1 端到端打通到可编译）
1. 新建 `main/boards/easyinput-v2/`：
   - `config.h`：引脚映射（遵循硬件合同）。
   - `config.json`：板型与目标。
   - `easyinput_v2_board.cc`：`EasyInputV2Board : WifiBoard`，含
     - `InitializePower()`：GPIO8 共享电源域严格时序（锁存低→拉高→延时 100ms 待实测）。
     - `InitializeAudio()`：`NoAudioCodecSimplex`（麦 GPIO9/10/11，扬声器 GPIO14/13/15 独立时钟）。
     - `InitializeLeds()`：`EasyInputV2Strip`（继承 `CircularStrip`，自定义 5 灯状态灯效）。
     - `InitializeButtons()`：S1(GPIO2) 主交互；`InitializeEncoder()`：Knob(17/16) 调音量 + 按压(18)=对话开/关。
     - `InitializeBattery()`：`AdcBatteryMonitor`（GPIO4=ADC1_CH3，分压 1:1，充电 GPIO39）。
     - `InitializeStatusLed()`：GPIO42 绿色状态灯。
     - 覆写 `GetAudioCodec/GetLed/GetBatteryLevel`，`DECLARE_BOARD`。
   - `README.md`。
2. 注册构建链：`main/Kconfig.projbuild` 加 `BOARD_TYPE_EASYINPUT_V2`；`main/CMakeLists.txt` 加 `elseif` 分支。

## 待办（下一步）
- [ ] **接 ESP-IDF v6.1 环境 + 实板**，跑 `python scripts/build.py easyinput-v2` 验证编译。
- [ ] 烧录并真机验证：配网、聆听/说话、灯效、音量、电池读数。
- [ ] 实测 GPIO8 拉高后电源稳定最短时间，回填 `InitializePower()` 与 `flow/decisions.md`。
- [ ] 扩展：S2–S8 动作、KEY_WAKE 唤醒、充电指示、MCP 灯控工具。

## 风险
- 无显示器：板子无屏，`GetDisplay()` 返回 nullptr，走无显示路径；字体仍设 14_1 保构建变量完整。
- 音频采样率/位宽/槽格式需实板验证（板子合同未定默认值）。
- 电池分压常开（v1 为简单正确），后续可按需闸门省电。

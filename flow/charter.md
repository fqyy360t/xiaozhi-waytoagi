# Charter — EasyInput V2 适配 xiaozhi-esp32

## 目标
把 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)（ESP-IDF v6.1 语音助手固件）移植到 **EasyInput V2.0** 开发板（ESP32-S3R8 / 8MB PSRAM / 16MB Flash / I2S 麦克风 + MAX98357A / 5×WS2812 / 8 键 + 编码器），让小智能在该板上语音对话，并用 5 颗 RGB 灯做状态指示。

## 范围（本项目的边界）
- **被改造仓库**：`xiaozhi-esp32`（独立 git 仓库，目标代码）。
- **只读参考（不改动）**：`easyinput-board-cy`（板子硬件合同/资料）、`easy-input-maker`（板子固件/maker 源码）。这两个是输入，不是本项目边界。
- 新增板型 `easyinput-v2`，唯一身份、独立 OTA 通道。

## 非目标
- 不修改任何已有板子的引脚或配置。
- 不在 `easyinput-board-cy` / `easy-input-maker` 内建立控制层或写代码。
- v1 不做：S2–S8 按键动作绑定、KEY_WAKE 深度睡眠唤醒、充电指示灯效、基于 MCP 的灯控工具、显示器。

## 关键约束（来自板子硬件合同，不可破坏）
- **GPIO0 是 BOOT0 下载入口，绝不作产品键。**
- **GPIO8 共享电源域**：LED/麦克风/扬声器都挂在它后面，必须先拉高并等电源稳定再初始化。
- **烧录**：开机态短按一次 BOOT 即进下载模式；不要「按住 BOOT 再上电」；板上无独立 RESET 键。
- 引脚事实以 `easyinput-board-cy/references/` 为准，不凭通用 ESP32 教程记忆。

## 成功标准
- `python scripts/build.py easyinput-v2` 可编译通过。
- 烧录后小智能语音对话（配网→待机→聆听→说话），5 颗 WS2812 按状态灯效变化。
- 编码器可调音量；电池电量可被读取。

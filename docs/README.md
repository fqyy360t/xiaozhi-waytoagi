# docs — EasyInput V2 适配 xiaozhi-esp32

本目录集中存放本次适配的跨模块知识（与 `flow/` 的推进/决策/交接区分开）。

## 板子资料
- [easyinput-v2 引脚映射](board/easyinput-v2-引脚映射.md) — 完整 GPIO 表、有效电平、板级合同红线
- [easyinput-v2 RGB 灯效设计](board/easyinput-v2-led-effects.md) — 5 颗 WS2812 的状态→灯效映射与设计说明

## 板型代码
- `main/boards/easyinput-v2/`：`config.h` / `config.json` / `easyinput_v2_board.cc` / `README.md`
- 构建注册：`main/Kconfig.projbuild`、`main/CMakeLists.txt`

## 权威来源
- xiaozhi 总文档：`README.md`、[custom-board.md](custom-board.md)、`main/audio/README.md`
- 板子硬件合同（只读参考，不在本项目边界）：`easyinput-board-cy/references/`（pinout / power-and-peripherals / boot-flash-recovery / identity-and-authority / known-gaps-and-errata）

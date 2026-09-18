# EasyInput V2 开发板

将 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 适配到 **EasyInput V2.0**（PCB 丝印 `AI Keyboard V2.1`，固件板型别名 `v2`）开发板。

## 硬件基线

| 项目 | 规格 |
| --- | --- |
| SoC | ESP32-S3R8（内置 8MB Octal SPI PSRAM） |
| Flash | 16MB（W25Q128 系列） |
| 无线 | 板载 Wi-Fi / BLE，PCB 天线 |
| USB | 原生 USB（GPIO19/20），USB-C，用于 ROM 下载 |
| 音频 | 数字 I2S 麦克风（GPIO9/10/11）+ MAX98357A I2S 功放（GPIO14/13/15） |
| 输入 | 8 路独立按键（S1–S8）、旋转编码器（A/B/按压）、KEY_WAKE 汇总唤醒 |
| 灯光 | 5 颗串联 WS2812（GPIO12）+ 独立绿色状态灯（GPIO42） |
| 电源 | 电池 + USB 供电，GPIO8 共享外设电源域使能 |

> 引脚事实来自 `easyinput-board-cy` 硬件合同，与通用 ESP32 教程无关。
> **GPIO0 是 BOOT0 下载入口，绝不作产品键。**

## 引脚映射

| 功能 | GPIO | 说明 |
| --- | --- | --- |
| 主交互键 S1 | 2 | 单击=对话开/关；开机态单击=进入配网 |
| S2–S8 | 47/38/41/1/6/7/48 | 低有效，板载上拉（v1 未绑定动作） |
| 编码器 A / B / 按压 | 17 / 16 / 18 | A/B 正交相位；按压=对话开/关 |
| 共享电源域使能 | 8 | 高有效，控制 LED/麦克风/扬声器供电 |
| WS2812 数据 | 12 | 5 颗串联，GRB |
| 绿色状态灯 | 42 | 系统就绪指示 |
| 麦克风 BCLK/WS/DIN | 9 / 10 / 11 | I2S 输入 |
| 扬声器 BCLK/WS/DOUT | 14 / 13 / 15 | I2S 输出（MAX98357A） |
| 电池分压使能/采样 | 5 / 4(ADC1_CH3) | VBAT/2 进入 ADC |
| 充电状态 / 外部供电 | 39 / 40 | 39 高=充电中；40 低=外部供电存在 |

## 构建与烧录

```sh
# 1) 进入 xiaozhi-esp32 并配置 ESP-IDF v6.1 环境
idf.py set-target esp32s3

# 2) 推荐用构建脚本（自动选择板型与配置）
python scripts/build.py easyinput-v2

# 3) 烧录（板子专属：开机状态短按一次 BOOT 即进入下载模式）
python scripts/build.py easyinput-v2 --flash

# 或手动：idf.py menuconfig -> Xiaozhi Assistant -> Board Type -> EasyInput V2
idf.py build
idf.py flash monitor
```

**烧录注意**：当前板为「开机态短按 BOOT 即下载」，不要使用「按住 BOOT 再上电」的旧流程。
板上没有独立 RESET 键；退出下载模式只需关机再开机。

## 灯效

5 颗 WS2812 的状态→灯效映射见 [`docs/board/easyinput-v2-led-effects.md`](../../docs/board/easyinput-v2-led-effects.md)。

## 已知限制 / 后续

- 共享电源域稳定时间（GPIO8 拉高后）尚未在实板测量，暂用 100ms 保守值。
- S2–S8 按键、KEY_WAKE 深度睡眠唤醒、充电指示、基于 MCP 的灯控工具待后续扩展。
- 音量通过编码器调节（`AudioCodec::SetOutputVolume`，0–100）。

# EasyInput V2 — 5 颗 WS2812 灯效设计

## 设计目标
- 用 5 颗串联 WS2812（GPIO12，GRB）直观表达小智设备状态。
- 复用 xiaozhi 现成 `CircularStrip`（RMT 驱动），仅通过子类 `EasyInputV2Strip` 覆盖 `OnStateChanged()` 实现自定义配色与动效，不改动框架。
- 配色区分度优先：开机/配网=冷色，聆听=白（录音中），说话=绿（输出中），错误=红。

## 状态 → 灯效映射
| 设备状态 | 颜色（GRB） | 动效 | 语义 |
| --- | --- | --- | --- |
| Starting（开机） | 青 `{0,48,48}` | Scroll（长度2, 120ms） | 上电扫光 |
| WifiConfiguring（配网） | 蓝 `{0,0,48}` | Blink（500ms） | 等待手机配网 |
| Idle（待机在线） | 冷白 `{4,10,20}` | Breathe（1200ms） | 轻微呼吸，低功耗氛围 |
| Connecting（连接服务器） | 蓝微光 `{0,0,12}` | SetAllColor 常亮 | 连接中 |
| Listening / AudioTesting（聆听） | 白 `{48,48,48}` | SetAllColor 常亮 | 录音中 |
| Speaking / Notifying（说话/通知） | 绿 `{0,48,0}` | SetAllColor 常亮 | 输出中 |
| Upgrading（OTA 升级） | 绿 `{0,48,0}` | Blink（120ms）快闪 | 升级中 |
| Activating（激活） | 绿 `{0,48,0}` | Blink（500ms）慢闪 | 激活中 |
| FatalError（致命错误） | 红 `{48,0,0}` | Blink（200ms）快闪 | 异常 |

## 实现要点
- `EasyInputV2Strip` 继承 `CircularStrip(GPIO_NUM_12, 5)`，覆写 `OnStateChanged()`。
- 应用每次状态变化自动回调 `OnStateChanged()`（见 `main/led/circular_strip.cc` 基类与 `main/application.cc`）。
- 亮度用字面量（BRIGHT=48 / DIM=12 / AMBIENT=20），不依赖基类私有常量。
- 灯带 GPIO12 在 `InitializePower()` 拉高 GPIO8 之后才构造（构造即发清屏帧），时序由板子硬件合同保证。

## 后续可选增强
- 充电时让某颗灯呼吸绿/橙（需接 `AdcBatteryMonitor::OnChargingStatusChanged`）。
- 用 `mcp_server` 暴露灯控工具（参考 `lamp_controller.h` 的 `self.lamp.*` 模式）。
- 按状态用不同长度的 Scroll/彩虹，增强辨识度。

# Decisions — 决策记录

| 日期 | 决策 | 理由 | 状态 |
| --- | --- | --- | --- |
| 2026-09-18 | 项目边界 = `xiaozhi-esp32` 独立仓库 | 它是被改造的目标代码；两个参考库只读取不改 | 已定 |
| 2026-09-18 | 板型标识 `easyinput-v2`，flat 布局 | 单板，不声明 manufacturer；符合命名规则（小写+连字符） | 已定 |
| 2026-09-18 | 音频用 `NoAudioCodecSimplex` | 麦(GPIO9/10/11)与扬声器(GPIO14/13/15)是独立 I2S 时钟线，非单总线 Duplex | 已定 |
| 2026-09-18 | 音频采样率 24000/24000 | 简单 I2S 麦+MAX98357A 常用值；实板验证 | 待验证 |
| 2026-09-18 | 灯带用 `CircularStrip(GPIO12,5)` + 子类 `EasyInputV2Strip` | 复用 xiaozhi 现成 WS2812(RMT)驱动，仅覆盖 `OnStateChanged` 做自定义灯效 | 已定 |
| 2026-09-18 | GPIO8 拉高后延时 100ms | 保守值；**最短稳定时间未在实板测量**，待实测回填 | 待实测 |
| 2026-09-18 | 主交互键 = S1(GPIO2)，编码器按压 = 对话开/关 | GPIO0 是 BOOT0 绝不作产品键；标准小智交互 | 已定 |
| 2026-09-18 | 编码器旋转调音量（`AudioCodec::SetOutputVolume`, 0–100） | 无独立音量键；Application 无音量 API，直接调 codec | 已定 |
| 2026-09-18 | 电池分压常开（GPIO5 拉高） | v1 为简单正确；后续可按需闸门省电 | 待优化 |
| 2026-09-18 | 不安装 CC/Codex 的 stop-doccheck hooks | 当前在 WorkBuddy，hooks 不触发；避免污染上游仓库 | 已定 |
| 2026-09-18 | 本版本不加屏幕（无 Display） | 板子硬件无屏幕；`EasyInputV2Board` 仅继承 `WifiBoard`，不实现 `GetDisplay`/`InitializeDisplay`，应用走无屏路径；后续若加屏幕再补 `display` 初始化 | 已定 |

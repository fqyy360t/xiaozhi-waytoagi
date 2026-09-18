#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ============================================================================
// EasyInput V2 (AI Keyboard V2.1 / 固件板型别名 v2)
// ESP32-S3R8, 8MB Octal PSRAM, 16MB Flash, native USB-C, I2S mic + MAX98357A
//
// 引脚事实来自 easyinput-board-cy (硬件合同)，不是通用 ESP32 教程。
// 严禁把 GPIO0 当作产品键：它是 BOOT0/下载入口。
// 麦克风/扬声器/WS2812/绿灯都挂在 GPIO8 共享电源域后面，必须先 Enable 再初始化。
// ============================================================================

// ---- 音频：NoAudioCodecSimplex（麦克风与扬声器使用各自独立的 I2S 时钟线）----
#define AUDIO_INPUT_SAMPLE_RATE   24000
#define AUDIO_OUTPUT_SAMPLE_RATE  24000
#define AUDIO_I2S_METHOD_SIMPLEX

// 麦克风 BCLK/WS/DIN = GPIO9/10/11
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_10
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_9
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_11

// 扬声器(MAX98357A) BCLK/WS/DOUT = GPIO14/13/15
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_15
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_14
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_13

// ---- 共享外设电源域使能（高有效），控制 LED/麦克风/扬声器供电 ----
#define SHARED_POWER_ENABLE_GPIO  GPIO_NUM_8

// ---- WS2812 灯带：5 颗串联，GPIO12，GRB ----
#define BUILTIN_LED_GPIO          GPIO_NUM_12
#define BUILTIN_LED_COUNT         5

// ---- 独立绿色状态灯（同样在 GPIO8 电源域内）----
#define STATUS_LED_GPIO           GPIO_NUM_42

// ---- 主交互键：S1（低有效）。GPIO0 是 BOOT0，绝不作产品键 ----
#define MAIN_BUTTON_GPIO          GPIO_NUM_2

// ---- 旋转编码器 A/B 与按压 S9 ----
#define ENCODER_A_GPIO            GPIO_NUM_17
#define ENCODER_B_GPIO            GPIO_NUM_16
#define ENCODER_PRESS_GPIO        GPIO_NUM_18

// ---- 电池 / 充电检测 ----
// SEN_VBAT=GPIO4(ADC1_CH3, 板上约 VBAT/2)；SEN_EN=GPIO5(分压使能, 高有效)
// SEN_CHRG=GPIO39(高=充电中, 仅外部供电时有效)；SEN_VIN=GPIO40(低=外部供电)
#define BATTERY_ADC_UNIT          ADC_UNIT_1
#define BATTERY_ADC_CHANNEL        ADC_CHANNEL_3
#define BATTERY_ENABLE_GPIO       GPIO_NUM_5
#define BATTERY_CHARGING_GPIO     GPIO_NUM_39
#define BATTERY_VIN_GPIO          GPIO_NUM_40

// ---- 其余 7 颗主按键（低有效，板载上拉；v1 暂未绑定动作，留作 MCP/App 扩展）----
// S2=47 S3=38 S4=41 S5=1 S6=6 S7=7 S8=48
// KEY_WAKE=GPIO21 为二极管 OR 汇总唤醒线，不直接标识按键来源，v1 不用于消抖。

#endif // _BOARD_CONFIG_H_

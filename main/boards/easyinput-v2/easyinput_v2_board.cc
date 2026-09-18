#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "application.h"
#include "button.h"
#include "knob.h"
#include "led/circular_strip.h"
#include "adc_battery_monitor.h"
#include "display.h"        // 提供 HAVE_LVGL 与 lvgl.h（本板无屏，但需要 lv_init）
#include "config.h"
#include "mcp_server.h"

#include <driver/i2s_std.h> // I2S_STD_SLOT_RIGHT/LEFT：EasyInput V2 麦克风数据落在右声道

#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "EasyInputV2Board"

// ============================================================================
// 5 颗 WS2812 的自定义灯效（EasyInput V2 配色方案）
// 应用每次设备状态变化都会回调 OnStateChanged()。
// 配色采用 GRB（WS2812 格式），亮度用字面量避免依赖基类私有常量。
// 完整设计见 docs/board/easyinput-v2-led-effects.md
// ============================================================================
static constexpr uint8_t BRIGHT = 48;   // 高亮
static constexpr uint8_t DIM    = 12;   // 微亮
static constexpr uint8_t AMBIENT = 20;  // 呼吸峰值

class EasyInputV2Strip : public CircularStrip {
public:
    EasyInputV2Strip() : CircularStrip(BUILTIN_LED_GPIO, BUILTIN_LED_COUNT) {}

    void OnStateChanged() override {
        auto& app = Application::GetInstance();
        auto state = app.GetDeviceState();
        StripColor off = {0, 0, 0};
        switch (state) {
            case kDeviceStateStarting:
                // 开机：青色扫光
                Scroll(off, {0, BRIGHT, BRIGHT}, 2, 120);
                break;
            case kDeviceStateWifiConfiguring:
                // 配网：蓝色慢呼吸式闪烁
                Blink({0, 0, BRIGHT}, 500);
                break;
            case kDeviceStateIdle:
                // 待机在线：冷白轻微呼吸
                Breathe(off, {4, AMBIENT / 2, AMBIENT}, 1200);
                break;
            case kDeviceStateConnecting:
                // 连接服务器中：蓝色常亮微光
                SetAllColor({0, 0, DIM});
                break;
            case kDeviceStateListening:
            case kDeviceStateAudioTesting:
                // 聆听（你在说话）：白色常亮 = 录音中
                SetAllColor({BRIGHT, BRIGHT, BRIGHT});
                break;
            case kDeviceStateSpeaking:
            case kDeviceStateNotifying:
                // 说话/通知：绿色常亮
                SetAllColor({0, BRIGHT, 0});
                break;
            case kDeviceStateUpgrading:
                // OTA 升级：绿色快闪
                Blink({0, BRIGHT, 0}, 120);
                break;
            case kDeviceStateActivating:
                // 激活：绿色慢闪
                Blink({0, BRIGHT, 0}, 500);
                break;
            case kDeviceStateFatalError:
                // 致命错误：红色快闪
                Blink({BRIGHT, 0, 0}, 200);
                break;
            default:
                ESP_LOGW(TAG, "Unknown led strip event: %d", state);
                return;
        }
    }
};

class EasyInputV2Board : public WifiBoard {
private:
    Button              main_button_{MAIN_BUTTON_GPIO};
    Knob                encoder_{ENCODER_A_GPIO, ENCODER_B_GPIO};
    Button              encoder_press_{ENCODER_PRESS_GPIO};
    NoAudioCodecSimplex* audio_codec_   = nullptr;
    EasyInputV2Strip*   led_strip_      = nullptr;
    AdcBatteryMonitor*  battery_monitor_ = nullptr;

    // 共享电源域：先锁存 GPIO8 为低，再拉高，等待电源稳定后才允许初始化
    // WS2812 / 麦克风 / 扬声器（这些外设都挂在该电源域后）。
    // 注意：稳定所需最短时间目前未在实板测量，100ms 为按器件手册的保守值，
    // 待实板测量后写入 flow/decisions.md。严禁从其他工程复制延时当板级最小值。
    void InitializePower() {
        gpio_config_t io = {};
        io.pin_bit_mask = (1ULL << SHARED_POWER_ENABLE_GPIO);
        io.mode = GPIO_MODE_OUTPUT;
        io.pull_up_en = GPIO_PULLUP_DISABLE;
        io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io));
        gpio_set_level(SHARED_POWER_ENABLE_GPIO, 0);  // 先锁存低
        gpio_set_level(SHARED_POWER_ENABLE_GPIO, 1);  // 使能共享域
        vTaskDelay(pdMS_TO_TICKS(100));               // 等待电源稳定（待实测）
    }

    void InitializeStatusLed() {
        gpio_config_t io = {};
        io.pin_bit_mask = (1ULL << STATUS_LED_GPIO);
        io.mode = GPIO_MODE_OUTPUT;
        io.pull_up_en = GPIO_PULLUP_DISABLE;
        io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io));
        gpio_set_level(STATUS_LED_GPIO, 1);  // 系统就绪指示
    }

    // 本板无屏（不实现显示接口，Board::GetDisplay() 走默认 NoDisplay），
    // 但小智的 Assets::LvglStrategy::Apply() 会先构造 LvglCBinFont（内部走
    // lv_malloc），之后才去判断 display 是否可用。若 LVGL 未初始化，TLSF 控制块
    // 为空指针，会在 starting -> activating 加载 assets 时触发 LoadProhibited 崩溃。
    // 因此即使没有屏幕，也必须先初始化 LVGL（只初始化内存/定时器，不创建显示设备）。
    void InitializeLvgl() {
#if HAVE_LVGL
        if (!lv_is_initialized()) {
            lv_init();
            ESP_LOGI(TAG, "LVGL initialized early (headless board, no display)");
        }
#endif
    }

    void InitializeButtons() {
        main_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

    void InitializeEncoder() {
        encoder_.OnRotate([this](bool clockwise) {
            if (audio_codec_ == nullptr) return;
            int v = audio_codec_->output_volume() + (clockwise ? 5 : -5);
            if (v < 0) v = 0;
            if (v > 100) v = 100;
            audio_codec_->SetOutputVolume(v);
        });
        encoder_press_.OnClick([this]() {
            Application::GetInstance().ToggleChatState();
        });
    }

    void InitializeAudio() {
        // 注意：EasyInput V2 的 I2S 麦克风把数据放在 WS 高电平（右声道），
        // 必须用 I2S_STD_SLOT_RIGHT 才能采到声；用默认 LEFT 会读到静音，
        // 表现为"喊你好小智没反应、按 S1 进聆听中说话也没回应"。
        // 出处：官方 easy-input-maker 固件 prepare_microphone_channel_locked()
        // 中 std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT（已实测验证）。
        // 扬声器(MAX98357A)保持默认左声道。
        audio_codec_ = new NoAudioCodecSimplex(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            I2S_STD_SLOT_LEFT,
            AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN,
            I2S_STD_SLOT_RIGHT);
    }

    void InitializeLeds() {
        // 必须在 InitializePower() 之后：CircularStrip 构造即发送清屏帧
        led_strip_ = new EasyInputV2Strip();
    }

    void InitializeBattery() {
        // 先配置分压使能脚为输出，再拉高（否则仅设电平不生效，VBAT 采样不到）
        gpio_config_t bat_io = {};
        bat_io.pin_bit_mask = (1ULL << BATTERY_ENABLE_GPIO);
        bat_io.mode = GPIO_MODE_OUTPUT;
        bat_io.pull_up_en = GPIO_PULLUP_DISABLE;
        bat_io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        bat_io.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&bat_io));
        // 拉高分压使能，让 SEN_VBAT 可采样（v1 常开；后续可按需闸门以省电）
        gpio_set_level(BATTERY_ENABLE_GPIO, 1);
        // 分压为 1:1，VBAT/2 进入 ADC；charging_pin 读取 SEN_CHRG
        battery_monitor_ = new AdcBatteryMonitor(
            BATTERY_ADC_UNIT, BATTERY_ADC_CHANNEL,
            100000.0f, 100000.0f, BATTERY_CHARGING_GPIO);
    }

public:
    EasyInputV2Board() :
        main_button_(MAIN_BUTTON_GPIO),
        encoder_(ENCODER_A_GPIO, ENCODER_B_GPIO),
        encoder_press_(ENCODER_PRESS_GPIO) {
        InitializePower();
        InitializeLvgl();     // 必须在 Assets::Apply() 之前；无屏也必须初始化
        InitializeStatusLed();
        InitializeButtons();
        InitializeEncoder();
        InitializeAudio();
        InitializeLeds();
        InitializeBattery();
    }

    virtual AudioCodec* GetAudioCodec() override {
        return audio_codec_;
    }

    virtual Led* GetLed() override {
        return led_strip_;
    }

    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        if (battery_monitor_ == nullptr) return false;
        charging = battery_monitor_->IsCharging();
        discharging = battery_monitor_->IsDischarging();
        level = battery_monitor_->GetBatteryLevel();
        return true;
    }

    virtual std::string GetBoardType() override {
        return "easyinput-v2";
    }
};

DECLARE_BOARD(EasyInputV2Board);
